
#version 330 core

#define MAX_BUFFER_SIZE 512

uniform int vertex_amount;
// the fragment
uniform vec3 vertex_buffer[MAX_BUFFER_SIZE];
uniform float z0;
uniform mat4 view_mat;
uniform vec3 light_pos;
//uniform int light_amount;
//uniform vec3 light_buffer[MAX_BUFFER_SIZE];
uniform vec3 cam_p;

in vec2 frag_pos;
out vec4 FragColor;

void main() {
    
    float lowest_dist = -1.0f;
    vec3 normalized_ray = normalize(vec3(frag_pos.x, frag_pos.y, z0));
    for (int i = 0; i < vertex_amount / 3; i++) {

        vec4 A_4 = view_mat * (vec4(vertex_buffer[i*3], 0.0f) - vec4(cam_p, 0.0f));
        vec4 B_4 = view_mat * (vec4(vertex_buffer[i*3+1], 0.0f) - vec4(cam_p, 0.0f));
        vec4 C_4 = view_mat * (vec4(vertex_buffer[i*3+2], 0.0f) - vec4(cam_p, 0.0f));

        vec3 A = vec3(A_4.x, A_4.y, A_4.z);
        vec3 B = vec3(B_4.x, B_4.y, B_4.z);
        vec3 C = vec3(C_4.x, C_4.y, C_4.z);

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
                }
            }

        }
        

    }

    if (lowest_dist != -1.0f) {
        
        vec3 ray_end = lowest_dist * normalized_ray;

        int gets_light = 1;

        vec4 transformed_light_pos_4 = view_mat * vec4(light_pos - cam_p, 0.0f);
        vec3 transformed_light_pos = vec3(transformed_light_pos_4.x, transformed_light_pos_4.y, transformed_light_pos_4.z);

        vec3 light_ray = transformed_light_pos - ray_end;
        vec3 normalized_light_ray = normalize(light_ray);

        float dist = length(light_ray);
        
        for (int j = 0; j < vertex_amount / 3; j++) {
            
            vec4 A_4 = view_mat * vec4(vertex_buffer[j*3] - cam_p, 0.0f) - vec4(ray_end, 0.0f);
            vec4 B_4 = view_mat * vec4(vertex_buffer[j*3+1] - cam_p, 0.0f) - vec4(ray_end, 0.0f);
            vec4 C_4 = view_mat * vec4(vertex_buffer[j*3+2] - cam_p, 0.0f) - vec4(ray_end, 0.0f);

            vec3 A = vec3(A_4.x, A_4.y, A_4.z);
            vec3 B = vec3(B_4.x, B_4.y, B_4.z);
            vec3 C = vec3(C_4.x, C_4.y, C_4.z);

            /*
            vec3 A = vertex_buffer[j*3] - ray_end;
            vec3 B = vertex_buffer[j*3+1] - ray_end;
            vec3 C = vertex_buffer[j*3+2] - ray_end;
            */

            vec3 normal = normalize(cross(B - A, C - A));

            float d = normal.x * A.x + normal.y * A.y + normal.z * A.z;
            float intersect_scalar = d / (normal.x * normalized_light_ray.x + normal.y * normalized_light_ray.y + normal.z * normalized_light_ray.z);
            
            // something weird happens in the following if statement which causes the shader to not work

            /*
            if (light_buffer[0].x == 1.0f) {
                int val = 2;
            }
            */
            

            
            if (intersect_scalar >= 0.1f && intersect_scalar <= dist) {
                
                vec3 intersect_point = normalized_light_ray * intersect_scalar;
                
                float triangle_area = length(cross(B - A, C - A));
                
                float area_a = length(cross(B - intersect_point, C - intersect_point));
                float area_b = length(cross(C - intersect_point, A - intersect_point));
                float area_c = length(cross(A - intersect_point, B - intersect_point));
                float area_sum = area_a + area_b + area_c;

                if (abs(triangle_area - area_sum) <= 0.01f) {
                    gets_light = 0;
                    break;
                }
                
            }
            
        }
        

        if (gets_light == 1) {
            FragColor = vec4(1.0f - lowest_dist*0.1f, 1.0f - lowest_dist*0.1f, 1.0f - lowest_dist*0.1f, 1.0f);
        } else {
            FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }

        //FragColor = vec4(1.0f - lowest_dist*0.1f, 1.0f - lowest_dist*0.1f, 1.0f - lowest_dist*0.1f, 1.0f);

    } else {
        FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    }

}
