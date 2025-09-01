#include <stdio.h>
#include <SDL.h>
#include <math.h>
#include <stdlib.h>

#define WIDTH 1920
#define HEIGHT 1080
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_GRAY 0xefefefef
#define COLOR_RAY 0xffd43b
#define COLOR_RAY_BLUR 0xbd6800
#define RAYS_NUMBER 500
#define RAY_THICKNESS 1
#define PI 3.14159265359
#define TARGET_FPS 60
#define FRAME_TIME (1000 / TARGET_FPS)

typedef struct {
    double x, y, r;
} Circle;

typedef struct {
    double start_x, start_y;
    double dx, dy;  // Direction vector instead of angle
} Ray;

// Safe pixel drawing with bounds checking
void safe_fill_rect(SDL_Surface* surface, int x, int y, int w, int h, Uint32 color) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        SDL_Rect rect = {x, y, w, h};
        SDL_FillRect(surface, &rect, color);
    }
}

// Efficient circle filling using scanline approach
void FillCircle(SDL_Surface* surface, Circle circle, Uint32 color) {
    int cx = (int)circle.x;
    int cy = (int)circle.y;
    int r = (int)circle.r;
    
    for (int y = -r; y <= r; y++) {
        int x_extent = (int)sqrt(r * r - y * y);
        for (int x = -x_extent; x <= x_extent; x++) {
            safe_fill_rect(surface, cx + x, cy + y, 1, 1, color);
        }
    }
}

void generate_rays(Circle light_source, Ray rays[RAYS_NUMBER]) {
    for (int i = 0; i < RAYS_NUMBER; i++) {
        double angle = ((double)i / RAYS_NUMBER) * 2 * PI;
        rays[i].start_x = light_source.x;
        rays[i].start_y = light_source.y;
        rays[i].dx = cos(angle);
        rays[i].dy = sin(angle);
    }
}

// Analytical ray-circle intersection
double intersect_ray_circle(Ray ray, Circle circle) {
    double dx = ray.start_x - circle.x;
    double dy = ray.start_y - circle.y;
    
    double a = ray.dx * ray.dx + ray.dy * ray.dy;
    double b = 2 * (dx * ray.dx + dy * ray.dy);
    double c = dx * dx + dy * dy - circle.r * circle.r;
    
    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return -1;  // No intersection
    
    double t1 = (-b - sqrt(discriminant)) / (2 * a);
    double t2 = (-b + sqrt(discriminant)) / (2 * a);
    
    // Return the closest positive intersection
    if (t1 > 0) return t1;
    if (t2 > 0) return t2;
    return -1;
}

// Check if ray hits screen boundary
double intersect_ray_screen(Ray ray) {
    double t_min = 0;
    double t_max = INFINITY;
    
    // Check intersection with screen boundaries
    if (ray.dx != 0) {
        double t1 = -ray.start_x / ray.dx;
        double t2 = (WIDTH - ray.start_x) / ray.dx;
        if (t1 > t2) { double temp = t1; t1 = t2; t2 = temp; }
        t_min = fmax(t_min, t1);
        t_max = fmin(t_max, t2);
    }
    
    if (ray.dy != 0) {
        double t1 = -ray.start_y / ray.dy;
        double t2 = (HEIGHT - ray.start_y) / ray.dy;
        if (t1 > t2) { double temp = t1; t1 = t2; t2 = temp; }
        t_min = fmax(t_min, t1);
        t_max = fmin(t_max, t2);
    }
    
    return (t_max >= t_min && t_max > 0) ? t_max : -1;
}

void FillRays(SDL_Surface* surface, Ray rays[RAYS_NUMBER], Uint32 color, Uint32 blur_color, Circle obstacle) {
    for (int i = 0; i < RAYS_NUMBER; i++) {
        Ray ray = rays[i];
        
        // Find intersection with obstacle
        double obstacle_t = intersect_ray_circle(ray, obstacle);
        
        // Find intersection with screen boundary
        double screen_t = intersect_ray_screen(ray);
        
        // Determine the maximum distance to draw
        double max_t = screen_t;
        if (obstacle_t > 0 && (screen_t < 0 || obstacle_t < screen_t)) {
            max_t = obstacle_t;
        }
        
        if (max_t <= 0) continue;  // No valid intersection
        
        // Draw ray using DDA-like approach
        double step_size = 2.0;
        int steps = (int)(max_t / step_size);
        
        for (int step = 0; step < steps; step++) {
            double t = step * step_size;
            if (t > max_t) break;
            
            int x = (int)(ray.start_x + t * ray.dx);
            int y = (int)(ray.start_y + t * ray.dy);
            
            // Draw blur effect
            double blur_size = 1.5 * RAY_THICKNESS;
            safe_fill_rect(surface, x - blur_size/2, y - blur_size/2, blur_size, blur_size, blur_color);
            
            // Draw main ray
            safe_fill_rect(surface, x, y, RAY_THICKNESS, RAY_THICKNESS, color);
        }
    }
}

int main() {
    // Initialize SDL with proper error checking
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return -1;
    }
    
    SDL_Window* window = SDL_CreateWindow("Improved Raytracing", 
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                        WIDTH, HEIGHT, 0);
    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    if (!surface) {
        fprintf(stderr, "Surface creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Initialize objects
    Circle light_source = {200, 200, 40};
    Circle shadow_circle = {550, 300, 140};
    SDL_Rect erase_rect = {0, 0, WIDTH, HEIGHT};
    
    // Pre-generate rays (will be updated when light moves)
    Ray rays[RAYS_NUMBER];
    generate_rays(light_source, rays);
    int rays_need_update = 0;
    
    // Animation parameters
    double t = 0;
    double ellipse_a = 480;
    double ellipse_b = 350;
    double ellipse_center_x = WIDTH / 2.0;
    double ellipse_center_y = HEIGHT / 2.0;
    double speed = 0.022;
    
    int simulation_running = 1;
    SDL_Event event;
    
    while (simulation_running) {
        Uint32 frame_start = SDL_GetTicks();
        
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                simulation_running = 0;
            }
            if (event.type == SDL_MOUSEMOTION && event.motion.state != 0) {
                light_source.x = event.motion.x;
                light_source.y = event.motion.y;
                rays_need_update = 1;
            }
        }
        
        // Update rays only when light position changes
        if (rays_need_update) {
            generate_rays(light_source, rays);
            rays_need_update = 0;
        }
        
        // Clear screen
        SDL_FillRect(surface, &erase_rect, COLOR_BLACK);
        
        // Render rays
        FillRays(surface, rays, COLOR_RAY, COLOR_RAY_BLUR, shadow_circle);
        
        // Render objects
        FillCircle(surface, light_source, COLOR_WHITE);
        FillCircle(surface, shadow_circle, COLOR_WHITE);
        
        // Update shadow circle position
        shadow_circle.x = ellipse_center_x + ellipse_a * cos(t);
        shadow_circle.y = ellipse_center_y + ellipse_b * sin(t);
        t += speed;
        
        // Update display
        SDL_UpdateWindowSurface(window);
        
        // Frame rate control
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < FRAME_TIME) {
            SDL_Delay(FRAME_TIME - frame_time);
        }
    }
    
    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}