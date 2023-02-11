/*
    Rotation of cube rendered to OLED Screen
    Elia Karrer, 2023
*/

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <math.h>

#define OLED_X  128
#define OLED_Y  64

Adafruit_SSD1306 oled(OLED_X, OLED_Y, &Wire, -1);


// Edge between vertices
typedef struct { uint32_t a, b; } edge;

// 3d-vertex
class v3
{
public:
    double x, y, z;

    v3(void) : x(0), y(0), z(0) {}
    v3(double nx, double ny, double nz) : x(nx), y(ny), z(nz) {}
};

// Vector addition
v3 operator + (const v3 &a, const v3 &b)
{
    return v3(a.x + b.x, a.y + b.y, a.z + b.z);
}

// Scalar multiplication
v3 operator * (const v3 &a, double b)
{
    return v3(a.x * b, a.y * b, a.z * b);
}

// Vertex rotation by xyz-angle
v3 rotate(v3 vector, v3 angle)
{
    v3 rotated;

    // Rotation X
    rotated.x = vector.x;
    rotated.y = vector.y * cos(angle.x) - vector.z * sin(angle.x);
    rotated.z = vector.y * sin(angle.x) + vector.z * cos(angle.x);
    vector = rotated;
    
    // Rotation Y
    rotated.x = vector.z * sin(angle.y) + vector.x * cos(angle.y);
    rotated.y = vector.y;
    rotated.z = vector.z * cos(angle.y) - vector.x * sin(angle.y);
    vector = rotated;
    
    // Rotation Z
    rotated.x = vector.x * cos(angle.z) - vector.y * sin(angle.z);
    rotated.y = vector.x * sin(angle.z) + vector.y * cos(angle.z);
    rotated.z = vector.z;

    return rotated;
}


// 3d object with vertices and edges
class object_3d
{
private:
    uint32_t vertices_n, edges_n;
    v3* projection;
    v3* vertices;
    edge* edges;

    void update_projection(void);
    void draw(void);

public:
    v3 rotation;
    double scale;

    object_3d(v3* new_vertices, edge* new_edges, uint32_t new_vertices_n, uint32_t new_edges_n)
    {
        vertices = new_vertices;
        edges = new_edges;
        vertices_n = new_vertices_n;
        edges_n = new_edges_n;
        projection = new v3[new_vertices_n];

        rotation = v3(0, 0, 0);
        scale = 1;
    }

    ~object_3d(void)
    {
        delete projection;
    }

    void render(void);
};


void object_3d::update_projection(void)
{
    uint32_t i;

    for(i = 0; i < vertices_n; i++)
    {
        projection[i] = vertices[i] * scale;
        projection[i] = rotate(projection[i], rotation);
    }
}

void object_3d::draw(void)
{
    uint32_t i;

    for(i = 0; i < edges_n; i++)
    {
        // Padding to middle of screen
        int x0 = (int) projection[edges[i].a].x + (OLED_X / 2);
        int x1 = (int) projection[edges[i].b].x + (OLED_X / 2);
        int y0 = (int) projection[edges[i].a].y + (OLED_Y / 2);
        int y1 = (int) projection[edges[i].b].y + (OLED_Y / 2);

        oled.drawLine(x0, y0, x1, y1, WHITE);
    }
}


void object_3d::render(void)
{
    update_projection();
    draw();
}



v3 cube_vertices[] = { {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1}, {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1} };
edge cube_edges[] = { {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7} };
object_3d cube(&cube_vertices[0], &cube_edges[0], sizeof(cube_vertices) / sizeof(v3), sizeof(cube_edges) / sizeof(edge));


void setup(void)
{
    oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    oled.setTextColor(WHITE);
    oled.setCursor(0, 0);

    cube.scale = 20;
}


void loop(void)
{
    oled.clearDisplay();
    
    cube.render();
    oled.display();

    cube.rotation.x += 0.05;
    cube.rotation.y += 0.05;
    cube.rotation.z += 0.05;
}
