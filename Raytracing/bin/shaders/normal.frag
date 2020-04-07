#version 450 core
out vec4 FragColor;

//Camera
uniform vec3 camEye;
uniform vec3 camFront;
uniform vec3 camUp;
uniform mat4 viewMat;

//Shapes
const int shapeCount = 2;
struct Shape
{
	int type;
	vec3 position;
	vec3 scale;
	vec3 rotation;
	
};

uniform Shape shapes[shapeCount];

//Operations
const int opCount = 1;
struct Operation
{
	int type;
	int indexA;
	int indexB;
};

uniform Operation operations[opCount];


//
uniform vec2 Resolution;
uniform float Time;

const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float EPSILON = 0.0001;


float intersectSDF(float distA, float distB)
{
	return max(distA,distB);
}

float unionSDF(float distA, float distB)
{
	return min(distA, distB);
}

float differenceSDF(float distA, float distB)
{
	return max(distA, -distB);
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

float sphereSDF(vec3 p, float radius)
{
	return length(p) - radius;
}

float shapeSDF(vec3 samplePoint,int index)
{
	switch(index)
	{
		case 0:
		{
			return sphereSDF(samplePoint, shapes[index].scale.x);
			
		}
		case 1:
		{
			return cubeSDF(samplePoint, shapes[index].scale);
			
		}
		case 2:
		{
			return torusSDF(samplePoint, shapes[index].scale.xy);
			
		}
		
	}
	
}

float sceneSDF(vec3 samplePoint)
{
	float opArray[opCount];
	
	for(int i = 0; i < opCount; i++)
	{
		switch(operations[i].type)
		{
			case 0:
			{
				opArray[i] = intersectSDF(shapeSDF(samplePoint,operations[i].indexA),shapeSDF(samplePoint,operations[i].indexB));
				break;
			}
			case 1:
			{
				opArray[i] = unionSDF(shapeSDF(samplePoint,operations[i].indexA),shapeSDF(samplePoint,operations[i].indexB));
				break;
			}
			case 2:
			{
				opArray[i] = differenceSDF(shapeSDF(samplePoint,operations[i].indexA),shapeSDF(samplePoint,operations[i].indexB));
				break;
			}
			
		}
	}
	
	return opArray[opCount - 1];
}

float raymarch(vec3 eye, vec3 dir, float start, float end)
{
	float depth = start;
	for(int i = 0; i < MAX_MARCHING_STEPS; i++)
	{
		float dist = sceneSDF(eye + depth * dir);
		if(dist < EPSILON)
		{
			return depth;
		}
		depth += dist;
		if(depth >= end)
		{
			return end;
		}
		
	}
	
	return end;
	
}

vec3 estimateRayDirection(float FOV, vec2 size, vec2 fragCoord)
{
	vec2 xy = fragCoord.xy - size / 2.0;
	float z = size.y / tan(radians(FOV) / 2.0);
	return normalize(vec3(xy,-z));
}

vec3 estimateNormal(vec3 p)
{
	return normalize(vec3(
        sceneSDF(vec3(p.x + EPSILON, p.y, p.z)) - sceneSDF(vec3(p.x - EPSILON, p.y, p.z)),
        sceneSDF(vec3(p.x, p.y + EPSILON, p.z)) - sceneSDF(vec3(p.x, p.y - EPSILON, p.z)),
        sceneSDF(vec3(p.x, p.y, p.z  + EPSILON)) - sceneSDF(vec3(p.x, p.y, p.z - EPSILON))
    ));
}

//This is basically glm::lookat()
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

vec3 phongLight(vec3 k_d, vec3 k_s, float alpha, vec3 p, vec3 eye,
                          vec3 lightPos, vec3 lightIntensity) {
    vec3 N = estimateNormal(p);
    vec3 L = normalize(lightPos - p);
    vec3 V = normalize(eye - p);
    vec3 R = normalize(reflect(-L, N));
    
    float dotLN = dot(L, N);
    float dotRV = dot(R, V);
    
    if (dotLN < 0.0) {
        // Light not visible from this point on the surface
        return vec3(0.0, 0.0, 0.0);
    } 
    
    if (dotRV < 0.0) {
        // Light reflection in opposite direction as viewer, apply only diffuse
        // component
        return lightIntensity * (k_d * dotLN);
    }
    return lightIntensity * (k_d * dotLN + k_s * pow(dotRV, alpha));
}

vec3 illumination(vec3 k_a, vec3 k_d, vec3 k_s, float alpha, vec3 point, vec3 eye)
{
	const vec3 ambientLight = 0.5 * vec3(1.0,1.0,1.0);
	vec3 color = ambientLight * k_a;
	
	vec3 light1Pos = vec3(4,2,4);
	
	vec3 light1Intensity = vec3(0.4,0.4,0.4);
	
	color += phongLight(k_d,k_s,alpha, point,eye, light1Pos,light1Intensity);
	
	return color;
	
}

void main()
{           
   vec3 viewDir = estimateRayDirection(45.0, Resolution, gl_FragCoord.xy);
   
   vec3 eye = camEye;
  
   mat4 viewToWorld = viewMatrix(camEye,camEye + camFront,vec3(0,1,0));
   
   vec3 worldDir = (viewToWorld * vec4(viewDir, 0.0)).xyz;
   
   float dist = raymarch(eye, worldDir, MIN_DIST, MAX_DIST);
   
   if(dist > MAX_DIST - EPSILON)
   {
	    FragColor = vec4(0,0,0,0);
   }
   else
   { 
	   
		vec3 p = eye + dist * worldDir;

		vec3 K_a = vec3(0.2, 0.2, 0.2);
		vec3 K_d = vec3(0.7, 0.2, 0.2);
		vec3 K_s = vec3(1.0, 1.0, 1.0);
		float shininess = 10.0;

		vec3 color = illumination(K_a, K_d, K_s, shininess, p, eye);

		FragColor = vec4(color, 1.0);
   }
  
  
   
   
   
   
   
}