#version 450 core
out vec4 FragColor;

//Camera
uniform vec3 camEye;
uniform vec3 camFront;
uniform vec3 camUp;


//Shapes

struct Shape
{
	int type;
	vec3 position;
	vec3 scale;
	vec3 rotation;
};
uniform Shape shapes[50];
uniform int shapeCount;

//Operations
uniform bool DoOperations;

struct Operation
{
	int type;
	int indexA;
	int indexB;
};
uniform Operation operations[50];
uniform int opCount;

uniform vec2 Resolution;


uniform float smoothFactor = 0.1;
uniform float twistFactor = 3;
uniform float bendFactor = 3;
uniform float displacementFactor = 3;

const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 300.0;
const float EPSILON = 0.0001;

//////////////////////////////////////
//CSG FUNCTIONS
//////////////////////////////////////
float smoothUnionSDF(float distA, float distB)
{
	float h = clamp( 0.5 + 0.5 * (distB - distA) / smoothFactor, 0.0, 1.0);
	return mix(distB, distA, h) - smoothFactor * h * (1.0 - h);
}

float smoothIntersectSDF(float distA, float distB)
{
	float h = clamp( 0.5 - 0.5 * (distB - distA) / smoothFactor, 0.0, 1.0);
	return mix(distB, distA, h) + smoothFactor * h * (1.0 - h);
}

float smoothDifferenceSDF(float distA, float distB)
{
	float h = clamp( 0.5 - 0.5 * (distB + distA) / smoothFactor, 0.0, 1.0);
	return mix(distB, -distA, h) + smoothFactor * h * (1.0 - h);
}

float unionSDF(float distA, float distB)
{
	return min(distA, distB);
}

float intersectSDF(float distA, float distB)
{
	return max(distA,distB);
}

float differenceSDF(float distA, float distB)
{
	return max(-distA, distB);
}

//////////////////////////////////////
//SDF FUNCTIONS
//////////////////////////////////////

float octahedronSDF(vec3 point, float s)
{
	
	point = abs(point);
	float m = point.x+point.y+point.z-s;
	vec3 q;
	if( 3.0*point.x < m )
		q = point.xyz;
	else if( 3.0*point.y < m ) 
		q = point.yzx;
	else if( 3.0*point.z < m ) 
		q = point.zxy;
	
	else return m * 0.57735027;
    
  float k = clamp(0.5*(q.z-q.y+s),0.0,s); 
  return length(vec3(q.x,q.y-s+k,q.z-k)); 
}

float cappedConeSDF(vec3 point, vec3 scale)
{
	vec2 q = vec2( length(point.xz), point.y );
	vec2 k1 = vec2(scale.z,scale.x);
	vec2 k2 = vec2(scale.z - scale.y,2.0 * scale.x);
	vec2 ca = vec2(q.x - min(q.x,(q.y < 0.0) ? scale.y:scale.z), abs(q.y) - scale.x);
	vec2 cb = q - k1 + k2 * clamp( dot(k1-q,k2)/dot(k2,k2), 0.0, 1.0 );
	float s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
	return s * sqrt( min(dot(ca,ca),dot(cb,cb)) );
}

float roundedCylinderSDF(vec3 point, vec3 scale)
{
	vec2 d = vec2( length(point.xz)-2.0* scale.x + scale.z , abs(point.y) - scale.y );
	return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - scale.z;
}

float cappedCylinderSDF(vec3 point, vec2 scale)
{
	vec2 d = abs(vec2(length(point.xz),point.y)) - scale;
	return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float capsuleSDF(vec3 point, vec2 scale)
{
	point.y -= clamp( point.y, 0.0, scale.x );
	return length( point ) - scale.y;
}

float triPrismSDF(vec3 point, vec2 scale)
{
	const float k = sqrt(3.0);
	scale.x *= 0.5 * k;
	point.xy /= scale.x;
	point.x = abs(point.x) - 1.0;
	point.y = point.y + 1.0/k;
	
	if( point.x+k * point.y>0.0 )
	{
		point.xy=vec2(point.x-k * point.y,-k * point.x-point.y)/2.0;
	}		
	
    point.x -= clamp( point.x, -2.0, 0.0 );
    float d1 = length(point.xy)*sign(-point.y)*scale.x;
    float d2 = abs(point.z)-scale.y;
	
    return length(max(vec2(d1,d2),0.0)) + min(max(d1,d2), 0.);
}

float hexPrismSDF(vec3 point, vec2 scale)
{
	const vec3 k = vec3(-0.8660254, 0.5, 0.57735);
	point = abs(point);
	point.xy -= 2.0*min(dot(k.xy,point.xy),0.0) * k.xy;
	vec2 d = vec2(length(point.xy-vec2(clamp(point.x,-k.z*scale.x,k.z*scale.x ),scale.x)) * sign(point.y - scale.x),
	point.z-scale.y);
	
	return min(max(d.x,d.y),0.0) + length(max(d,0.0));
	
}

float roundBoxSDF(vec3 point, vec3 scale)
{
	vec3 d = abs(point) - scale;
	
	float insideD = min(max(d.x,max(d.y,d.z)),0.0) - smoothFactor;
	float outsideD = length(max(d,0.0));
	
	return insideD + outsideD;
}

float torusSDF(vec3 point, vec2 t)
{
	vec2 q = vec2(length(point.xz) - t.x,point.y);
	return length(q) - t.y;
}

float cubeSDF(vec3 point, vec3 scale)
{
	vec3 d = abs(point) - scale;
	
	float insideD = min(max(d.x,max(d.y,d.z)),0.0);
	float outsideD = length(max(d,0.0));
	
	return insideD + outsideD;
}

float sphereSDF(vec3 point, float radius)
{
	return length(point) - radius;
}

//////////////////////////////////////
//Rotation matrix along X axis
//////////////////////////////////////
mat3 rotateX(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(1, 0, 0),
        vec3(0, c, -s),
        vec3(0, s, c)
    );
}

//////////////////////////////////////
//Rotation matrix along Y axis
//////////////////////////////////////
mat3 rotateY(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, 0, s),
        vec3(0, 1, 0),
        vec3(-s, 0, c)
    );
}

//////////////////////////////////////
//Rotation matrix along Z axis
//////////////////////////////////////
mat3 rotateZ(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, -s, 0),
        vec3(s, c, 0),
        vec3(0, 0, 1)
    );
}

//////////////////////////////////////
//Concatenation of the three 
//rotaton matrices
//////////////////////////////////////
mat3 getRotation(int index)
{
	
	vec3 rot = shapes[index].rotation;
	
	return rotateX(rot.x) * rotateY(rot.y) * rotateZ(rot.z);
}

//////////////////////////////////////
//Return SDF of the given index
//////////////////////////////////////
float shapeSDF(vec3 samplePoint,int index)
{
	mat3 M2W = getRotation(index);
	//Apply inverse transformations
	vec3 pos = inverse(M2W) * ( samplePoint - shapes[index].position) ;
	int type = shapes[index].type;
	vec3 size = shapes[index].scale;
	
	switch(type)
	{
		case 0:
		{
			return sphereSDF(pos, size.x);
			
		}
		case 1:
		{
			return cubeSDF(pos, size);
			
		}
		case 2:
		{
			return torusSDF(pos, size.xy);
			
		}
		case 3:
		{
			return roundBoxSDF(pos,size);
		}
		case 4:
		{
			return hexPrismSDF(pos,size.xy);
		}
		case 5:
		{
			return triPrismSDF(pos,size.xy);
		}
		case 6:
		{
			return capsuleSDF(pos,size.xy);
		}
		case 7:
		{
			return cappedCylinderSDF(pos,size.xy);
		}
		case 8:
		{
			return roundedCylinderSDF(pos,size);
		}
		case 9:
		{
			return cappedConeSDF(pos,size);
		}
		case 10:
		{
			return octahedronSDF(pos,size.x);
		}
		
		
	}
	
	
	
}
//////////////////////////////////////
//Return SDF of the given index,
//no transformation applied,
//used for SDF alterations
//////////////////////////////////////
float shapeSDFNoTransform(vec3 pos,int index)
{
	int type = shapes[index].type;
	vec3 size = shapes[index].scale;
	
	switch(type)
	{
		case 0:
		{
			return sphereSDF(pos, size.x);
			
		}
		case 1:
		{
			return cubeSDF(pos, size);
			
		}
		case 2:
		{
			return torusSDF(pos, size.xy);
			
		}
		case 3:
		{
			return roundBoxSDF(pos,size);
		}
		case 4:
		{
			return hexPrismSDF(pos,size.xy);
		}
		case 5:
		{
			return triPrismSDF(pos,size.xy);
		}
		case 6:
		{
			return capsuleSDF(pos,size.xy);
		}
		case 7:
		{
			return cappedCylinderSDF(pos,size.xy);
		}
		case 8:
		{
			return roundedCylinderSDF(pos,size);
		}
		case 9:
		{
			return cappedConeSDF(pos,size);
		}
		case 10:
		{
			return octahedronSDF(pos,size.x);
		}
		
		
	}
	
	
	
}
//////////////////////////////////////
//Render SDFs and apply operations 
//or alterations
//////////////////////////////////////
float getSDF(vec3 samplePoint,int index, int type)
{
	mat3 M2W = getRotation(index);
	vec3 pos = inverse(M2W) * ( samplePoint - shapes[index].position) ;
	
	
	switch(type)
	{
		//Displacement
		case 0:
		{
			float d1 = shapeSDFNoTransform(pos,index);
			//Displacement Function
			float d2 = sin(displacementFactor * pos.x) * sin(displacementFactor * pos.y) 
			* sin(displacementFactor * pos.z);
			return d1 + d2;
			
		}
		//Twist
		case 1:
		{
			float k = twistFactor;
			float c = cos(k * pos.y);
			float s = sin(k * pos.y);
			mat2 m = mat2(c,-s,s,c);
			vec3 q = vec3(m*pos.xz,pos.y);
			return shapeSDFNoTransform(q,index);
			
		}
		//Bend
		case 2:
		{
			float k = bendFactor;
			float c = cos(k * pos.x);
			float s = sin(k * pos.x);
			mat2 m = mat2(c,-s,s,c);
			vec3 q = vec3(m*pos.xy,pos.z);
			return shapeSDFNoTransform(q,index);
			
		}
		//Infinite Repetition
		case 3:
		{
			const vec3 rep = vec3(3,3,3);
			vec3 q = mod(pos+0.5*rep,rep) - 0.5*rep;
			return shapeSDFNoTransform(q,index);
		}
		//No Alterations
		case -1:
		{
			return shapeSDFNoTransform(pos,index);
		}
		
	}
	
}


float sceneSDF(vec3 samplePoint)
{
	if(DoOperations)
	{
		float opArray[99];
	
		for(int i = 0; i < opCount; i++)
		{
			//Check if operation is an alteration
			if(operations[i].type >= 6)
			{
				opArray[i] = getSDF(samplePoint,operations[i].indexA,operations[i].type - 6);
			}
			//Else it's a CSG operation
			else
			{
				//Get indices
				float left, right;
				if(operations[i].indexA < 0)
				{
					left = opArray[abs(operations[i].indexA) - 1];
				}
				else
				{
					left = shapeSDF(samplePoint,operations[i].indexA);
				}
				
				if(operations[i].indexB < 0)
				{
					right = opArray[abs(operations[i].indexB) - 1];
				}
				else
				{
					right = shapeSDF(samplePoint,operations[i].indexB);
				}
				//Apply operation
				switch(operations[i].type)
				{
					case 0:
					{
						opArray[i] = intersectSDF(left,right);
						break;
					}
					case 1:
					{
						opArray[i] = unionSDF(left,right);
						break;
					}
					case 2:
					{
						opArray[i] = differenceSDF(left,right);
						break;
					}
					case 3:
					{
						opArray[i] = smoothIntersectSDF(left,right);
						break;
					}
					case 4:
					{
						opArray[i] = smoothUnionSDF(left,right);
						break;
					}
					case 5:
					{
						opArray[i] = smoothDifferenceSDF(left,right);
						break;
					}
					
				}
			}
		}
		//Get last operation as final result
		return opArray[opCount - 1];
	
	}
	//Else NO OPERATIONS
	else
	{
		float t = MAX_DIST;
		for(int i = 0; i < shapeCount; i++)
		{
			float current_t = shapeSDF(samplePoint,i);
			
			if( current_t < t)
				t = current_t;
		}
		
		return t;
	}
	
	
	
}
//////////////////////////////////////
//Raymarching
//////////////////////////////////////
float raymarch(vec3 eye, vec3 dir)
{
	float depth = MIN_DIST;
	for(int i = 0; i < MAX_MARCHING_STEPS; i++)
	{
		float dist = sceneSDF(eye + depth * dir);
		if(dist < EPSILON)
		{
			return depth;
		}
		depth += dist;
		if(depth >= MAX_DIST)
		{
			return MAX_DIST;
		}
		
	}
	
	return MAX_DIST;
	
}

//////////////////////////////////////
//Estimate the direction of the ray
//////////////////////////////////////
vec3 estimateRayDirection(float FOV)
{
	vec2 xy = gl_FragCoord.xy - Resolution / 2.0;
	float z = Resolution.y / tan(radians(FOV) / 2.0);
	return normalize(vec3(xy,-z));
}

//////////////////////////////////////
//Estimate the normal of the current point
//////////////////////////////////////
vec3 estimateNormal(vec3 p)
{
	return normalize(vec3(
        sceneSDF(vec3(p.x + EPSILON, p.y, p.z)) - sceneSDF(vec3(p.x - EPSILON, p.y, p.z)),
        sceneSDF(vec3(p.x, p.y + EPSILON, p.z)) - sceneSDF(vec3(p.x, p.y - EPSILON, p.z)),
        sceneSDF(vec3(p.x, p.y, p.z  + EPSILON)) - sceneSDF(vec3(p.x, p.y, p.z - EPSILON))
    ));
}

//////////////////////////////////////
//GLM LOOK AT, create view matrix
//////////////////////////////////////
mat4 viewMatrix(vec3 eye, vec3 center, vec3 up)
{
	vec3 f = normalize(center - eye);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);
    return mat4(
        vec4(s, 0.0),
        vec4(u, 0.0),
        vec4(-f, 0.0),
        vec4(0.0, 0.0, 0.0, 1)
    );
}
//////////////////////////////////////
//Phong Light model
//////////////////////////////////////
vec3 phongLight(vec3 k_d, vec3 k_s, float alpha, vec3 p, vec3 eye, vec3 lightPos, vec3 lightIntensity) 
{
    vec3 N = estimateNormal(p);
    vec3 L = normalize(lightPos - p);
    vec3 V = normalize(eye - p);
    vec3 R = normalize(reflect(-L, N));
    
    float dotLN = dot(L, N);
    float dotRV = dot(R, V);
	
    //Light is not visible
    if (dotLN < 0.0) 
        return vec3(0.0, 0.0, 0.0);
   
    //Light reflection in opposite direction as viewer, apply only diffuse
    if (dotRV < 0.0) 
        return lightIntensity * (k_d * dotLN);
	
    return lightIntensity * (k_d * dotLN + k_s * pow(dotRV, alpha));
}

//////////////////////////////////////
//Lights in the scene
//////////////////////////////////////
vec3 illumination(vec3 k_a, vec3 k_d, vec3 k_s, float alpha, vec3 point, vec3 eye)
{
	const vec3 ambientLight = 0.5 * vec3(1.0,1.0,1.0);
	vec3 color = ambientLight * k_a;
	
	vec3 light1Pos = vec3(5,5,5);
	
	vec3 light1Intensity = vec3(0.4,0.4,0.4);
	
	color += phongLight(k_d,k_s,alpha, point,eye, light1Pos,light1Intensity);
	
	return color;
	
}

//////////////////////////////////////
//Main
//////////////////////////////////////
void main()
{           
   //Get view direction
   vec3 viewDir = estimateRayDirection(45.0);
   //Get ray origin (position of camera)
   vec3 rayOrigin = camEye;
   //Get view matrix
   mat4 view = viewMatrix(camEye,camEye + camFront,camUp);
   //Get ray direction
   vec3 rayDir = (view * vec4(viewDir, 0.0)).xyz;
   
   float dist = raymarch(rayOrigin, rayDir);
   
   
   //No SDF encountered
   if(dist > MAX_DIST - EPSILON)
   {
	    FragColor = vec4(0,0,0,0);
   }
   //Calculate light
   else
   { 
	   
		vec3 p = rayOrigin + dist * rayDir;

		vec3 K_a = vec3(0.2, 0.2, 0.2);
		vec3 K_d = vec3(0.7, 0.2, 0.2);
		vec3 K_s = vec3(1.0, 1.0, 1.0);
		float shininess = 10.0;

		vec3 color = illumination(K_a, K_d, K_s, shininess, p, rayOrigin);

		FragColor = vec4(color, 1.0);
   }
  
  
   
   
   
   
   
}