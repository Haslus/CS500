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

uniform float smoothFactor = 0.1;
uniform vec2 Resolution;

const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 300.0;
const float EPSILON = 0.0001;

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
	vec3 pos = inverse(M2W) * (samplePoint - shapes[index].position);
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
//Render each shape of the scene
//////////////////////////////////////
float sceneSDF(vec3 samplePoint)
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

//////////////////////////////////////
//AABB that encapsulates what to render
//////////////////////////////////////
vec2 iBox( in vec3 ro, in vec3 rd, in vec3 rad )
{
    vec3 m = 1.0/rd;
    vec3 n = m*ro;
    vec3 k = abs(m)*rad;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
	return vec2( max( max( t1.x, t1.y ), t1.z ),
	             min( min( t2.x, t2.y ), t2.z ) );
}

//////////////////////////////////////
//Raymarching
//////////////////////////////////////
vec2 castRay(vec3 rayOrigin, vec3 rayDir)
{
	vec2 res = vec2(-1.0,-1.0);

	float tmin = MIN_DIST;
	float tmax = MAX_DIST;

	//floor
	float tp1 = (0.0-rayOrigin.y)/rayDir.y;
    if( tp1>0.0 )
    {
        tmax = min( tmax, tp1 );
        res = vec2( tp1, 1.0 );
    }
	//Check if it's inside renderable box
	vec2 tb = iBox( rayOrigin, rayDir, vec3(50,50,50) );
    if( tb.x<tb.y && tb.y>0.0 && tb.x<tmax)
    {
		//Raymarch
        tmin = max(tb.x,tmin);
        tmax = min(tb.y,tmax);
		float t = tmin;
		for(int i = 0; i < MAX_MARCHING_STEPS; i++)
		{
			float dist = sceneSDF(rayOrigin + rayDir * t);
			if(dist < EPSILON)
			{
				res = vec2(t,55.0);
				return res;
			}
			t += dist;
			if(t >= tmax)
			{
				return res;
			}

		}
	}



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
//Hard Shadow
//////////////////////////////////////
float shadow(vec3 ro, vec3 rd, float mint, float maxt )
{

    for( float t=mint; t<maxt; )
    {
        float h = sceneSDF(ro + rd*t);
        if( h<0.001 )
            return 0.0;
        t += h;
    }

    return 1.0;
}
//////////////////////////////////////
//Render Floor (Taken from Inigo Quirez)
//////////////////////////////////////
float checkersGradBox( vec2 p, vec2 dpdx, vec2 dpdy )
{
    // filter kernel
    vec2 w = abs(dpdx)+abs(dpdy) + 0.001;
    // analytical integral (box filter)
    vec2 i = 2.0*(abs(fract((p-0.5*w)*0.5)-0.5)-abs(fract((p+0.5*w)*0.5)-0.5))/w;
    // xor pattern
    return 0.5 - 0.5*i.x*i.y;
}

//////////////////////////////////////
//Render scene
//////////////////////////////////////
vec3 render(vec3 RO, vec3 RD, vec3 RDX, vec3 RDY)
{
	vec3 col = vec3(0.7, 0.7, 0.9) - max(RD.y,0.0)*0.3;
	vec2 res = castRay(RO,RD);

	float t = res.x;
	float m = res.y;
	if( m > -0.5 )
    {

		vec3 pos = RO + t * RD;
		vec3 nor = (m<1.5) ? vec3(0.0,1.0,0.0) : estimateNormal( pos );
		vec3 ref = reflect( RD, nor);

		// material
		//col = 0.2 + 0.18*sin( vec3(0.05,0.08,0.10)*(m-1.0) );
        //col = vec3(0.2);
        col = 0.2 + 0.18*sin( m*2.0 + vec3(0.0,0.5,1.0) );

		//Draw infinite floor for the scene
		if(m<1.5 )
        {
            // project pixel footprint into the plane
            vec3 dpdx = RO.y*(RD/RD.y-RDX/RDX.y);
            vec3 dpdy = RO.y*(RD/RD.y-RDY/RDY.y);

            float f = checkersGradBox( 5.0*pos.xz, 5.0*dpdx.xz, 5.0*dpdy.xz );
            col = 0.15 + f*vec3(0.05);
        }

		//Calculate light components (Taken from Inigo Quirez)
		vec3  lig = normalize( vec3(-0.5, 0.4, -0.6) );
        vec3  hal = normalize( lig-RD );
		float amb = sqrt(clamp( 0.5+0.5*nor.y, 0.0, 1.0 ));
        float dif = clamp( dot( nor, lig ), 0.0, 1.0 );
        float bac = clamp( dot( nor, normalize(vec3(-lig.x,0.0,-lig.z))), 0.0, 1.0 )*clamp( 1.0-pos.y,0.0,1.0);
        float dom = smoothstep( -0.2, 0.2, ref.y );
        float fre = pow( clamp(1.0+dot(nor,RD),0.0,1.0), 2.0 );

		//Calcualte shadow for the floor and reflection
        dif *= shadow( pos, lig, 0.02, MAX_DIST);
        dom *= shadow( pos, ref, 0.02, MAX_DIST);

		float spe = pow( clamp( dot( nor, hal ), 0.0, 1.0 ),16.0)*
                    dif *
                    (0.04 + 0.96*pow( clamp(1.0+dot(hal,RD),0.0,1.0), 5.0 ));

		vec3 lin = vec3(0.0);
        lin += 3.80*dif*vec3(1.30,1.00,0.70);
        lin += 0.55*amb*vec3(0.40,0.60,1.15);
        lin += 0.85*dom*vec3(0.40,0.60,1.30);
        lin += 0.55*bac*vec3(0.25,0.25,0.25);
        lin += 0.25*fre*vec3(1.00,1.00,1.00);
		col = col*lin;
		col += 7.00*spe*vec3(1.10,0.90,0.70);


        col = mix( col, vec3(0.7,0.7,0.9), 1.0-exp( -0.000001*t*t*t ) );
    }

	return vec3( clamp(col,0.0,1.0) );


}

//////////////////////////////////////
//Main
//////////////////////////////////////
void main()
{
	vec3 viewDir = estimateRayDirection(45.0);

	vec3 rayOrigin = camEye;

	mat4 viewToWorld = viewMatrix(camEye,camEye + camFront,camUp);

	vec3 rayDir = normalize((viewToWorld * vec4(normalize(viewDir), 0.0)).xyz);

	//Calculate derivatives for rendering the floor
	vec2 px = (2.0*(gl_FragCoord.xy + vec2(1.0,0.0))-Resolution.xy)/Resolution.y;
	vec2 py = (2.0*(gl_FragCoord.xy + vec2(0.0,1.0))-Resolution.xy)/Resolution.y;
	vec3 rdx = vec3(viewToWorld * vec4(normalize( vec3(px,viewDir.z) ),0));
	vec3 rdy = vec3(viewToWorld * vec4(normalize( vec3(py,viewDir.z) ),0));

	vec3 color = render(rayOrigin, rayDir, rdx, rdy);

	//Gamma
	color = pow( color, vec3(0.4545) );

	FragColor = vec4(color, 1.0);







}