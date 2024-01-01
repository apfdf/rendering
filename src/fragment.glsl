

#version 330 core

#define MAX_BUFFER_SIZE 512

uniform int vertex_amount;
// the fragment shader expects the vertices to already be transformed by the main program
uniform vec3 vertex_buffer[MAX_BUFFER_SIZE];
uniform float z0;
uniform vec3 light_pos;
//uniform int light_amount;
//uniform vec3 light_buffer[MAX_BUFFER_SIZE];

in vec2 frag_pos;
out vec4 FragColor;

void main() {
    
    float lowest_dist = -1.0f;
    int intersect_triangle_index = -1;
    vec3 normalized_ray = normalize(vec3(frag_pos.x, frag_pos.y, z0));
    for (int i = 0; i < vertex_amount / 3; i++) {

        vec3 A = vertex_buffer[i*3];
        vec3 B = vertex_buffer[i*3+1];
        vec3 C = vertex_buffer[i*3+2];

        vec3 normal = normalize(cross(B - A, C - A));

        float d = normal.x * A.x + normal.y * A.y + normal.z * A.z;

        float intersect_scalar = d / (normal.x * normalized_ray.x + normal.y * normalized_ray.y + normal.z * normalized_ray.z);
        vec3 intersection = intersect_scalar * normalized_ray;
        if (intersect_scalar > 0.0f) {

            float triangle_area = length(cross(B - A, C - A));
            float area_c = length(cross(A - intersection, B - intersection));
            float area_a = length(cross(B - intersection, C - intersection));
            float area_b = length(cross(C - intersection, A - intersection));

            if (abs(area_a + area_b + area_c - triangle_area) <= 0.001f) {
                if (intersect_scalar < lowest_dist || lowest_dist == -1.0f) {
                    lowest_dist = intersect_scalar;
                    intersect_triangle_index = i;
                }
            }

        }
        

    }

    if (lowest_dist != -1.0f) {
        
        float brightness = 0.2f;

        int gets_light = 1;

        vec3 ray_end = lowest_dist * normalized_ray;
        vec3 light_ray = light_pos - ray_end;
        vec3 normalized_light_ray = normalize(light_ray);
        float dist = length(light_ray);

        vec3 triangle_normal = normalize(cross(vertex_buffer[intersect_triangle_index*3+1] - vertex_buffer[intersect_triangle_index*3], vertex_buffer[intersect_triangle_index*3+2] - vertex_buffer[intersect_triangle_index*3]));
        if (dot(triangle_normal, normalized_ray) > 0.0f) {
            triangle_normal = -triangle_normal;
        }
        
        if (dot(triangle_normal, normalized_light_ray) < 0.0f) {
            gets_light = 0;
        }

        for (int j = 0; j < vertex_amount / 3; j++) {
            
            if (j == intersect_triangle_index) {
                continue;
            }

            vec3 A = vertex_buffer[j*3] - ray_end;
            vec3 B = vertex_buffer[j*3+1] - ray_end;
            vec3 C = vertex_buffer[j*3+2] - ray_end;

            vec3 normal = normalize(cross(B - A, C - A));

            float d = normal.x * A.x + normal.y * A.y + normal.z * A.z;
            float intersect_scalar = d / (normal.x * normalized_light_ray.x + normal.y * normalized_light_ray.y + normal.z * normalized_light_ray.z);
            
            if (intersect_scalar >= 0.01f && intersect_scalar <= dist) {
                
                vec3 intersection = normalized_light_ray * intersect_scalar;
                
                float triangle_area = length(cross(B - A, C - A));
                
                float area_a = length(cross(B - intersection, C - intersection));
                float area_b = length(cross(C - intersection, A - intersection));
                float area_c = length(cross(A - intersection, B - intersection));
                float area_sum = area_a + area_b + area_c;

                if (abs(triangle_area - area_sum) <= 0.0001f) {
                    gets_light = 0;
                    break;
                }
                
            }
            
        }
        

        if (gets_light == 1) {
            vec3 triangle_normal = normalize(cross(vertex_buffer[intersect_triangle_index*3+1] - vertex_buffer[intersect_triangle_index*3], vertex_buffer[intersect_triangle_index*3+2] - vertex_buffer[intersect_triangle_index*3]));
            if (dot(triangle_normal, normalized_ray) > 0.0f) {
                triangle_normal = -triangle_normal;
            }
            brightness += (1 / (dist*dist)) * dot(triangle_normal, normalized_light_ray);
        }

        FragColor = vec4(brightness, brightness, brightness, 1.0f);

        //FragColor = vec4(1.0f - lowest_dist*0.1f, 1.0f - lowest_dist*0.1f, 1.0f - lowest_dist*0.1f, 1.0f);

    } else {
        FragColor = vec4(0.1f, 0.4f, 0.6f, 1.0f);
    }

}

