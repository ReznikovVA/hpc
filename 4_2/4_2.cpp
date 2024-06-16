#include <iostream>
#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <omp.h>

const int WIDTH = 800;
const int HEIGHT = 800;
const int MAX_ITERATIONS = 1000;
const double SCALE = 0.005;
const double OFFSET_X = -0.5;
const double OFFSET_Y = 0.0;

std::vector<std::vector<double>> mandelbrotData(WIDTH, std::vector<double>(HEIGHT, 0.0));

int mandelbrot(double x0, double y0) {
    double x = 0.0, y = 0.0;
    int iteration = 0;
    while (x * x + y * y <= 4.0 && iteration < MAX_ITERATIONS) {
        double xtemp = x * x - y * y + x0;
        y = 2 * x * y + y0;
        x = xtemp;
        ++iteration;
    }
    return iteration;
}

void computeMandelbrot() {
    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double x0 = SCALE * (x - WIDTH / 2) + OFFSET_X;
            double y0 = SCALE * (HEIGHT / 2 - y) + OFFSET_Y;
            int iteration = mandelbrot(x0, y0);
            double brightness = static_cast<double>(MAX_ITERATIONS - iteration) / MAX_ITERATIONS;
            mandelbrotData[x][y] = brightness;
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_POINTS);
    for (int x = 0; x < WIDTH; ++x) {
        for (int y = 0; y < HEIGHT; ++y) {
            double brightness = mandelbrotData[x][y];
            glColor3d(brightness, brightness, brightness);
            glVertex2i(x, y);
        }
    }
    glEnd();
    glFlush();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    int num_threads = omp_get_max_threads();
    std::cout << "Number of hardware threads: " << num_threads << std::endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Mandelbrot Set");
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    
    computeMandelbrot();

    glutMainLoop();
    
    return 0;
}
