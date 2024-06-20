#include <iostream>
#include <vector>
#include <complex>
#include <omp.h>
#include <GL/glut.h>

const int WIDTH = 800;
const int HEIGHT = 800;
const int MAX_ITERATIONS = 1000;
const double TOLERANCE = 1e-6;
const std::complex<double> ROOTS[] = { {1, 0}, {-0.5, sqrt(3) / 2}, {-0.5, -sqrt(3) / 2} };
const int NUM_ROOTS = sizeof(ROOTS) / sizeof(ROOTS[0]);

std::vector<std::vector<int>> newtonData(WIDTH, std::vector<int>(HEIGHT, 0));

int findRoot(const std::complex<double>& z) {
    for (int i = 0; i < NUM_ROOTS; ++i) {
        if (abs(z - ROOTS[i]) < TOLERANCE) {
            return i;
        }
    }
    return -1;
}

void computeNewtonFractal() {
    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            std::complex<double> z(
                4.0 * (x - WIDTH / 2) / WIDTH,
                4.0 * (HEIGHT / 2 - y) / HEIGHT
            );
            int iterations = 0;
            while (iterations < MAX_ITERATIONS) {
                std::complex<double> f = pow(z, 3) - 1.0;
                std::complex<double> f_prime = 3.0 * pow(z, 2);
                std::complex<double> z_next = z - f / f_prime;
                if (abs(z_next - z) < TOLERANCE) {
                    break;
                }
                z = z_next;
                ++iterations;
            }
            newtonData[x][y] = findRoot(z);
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_POINTS);
    for (int x = 0; x < WIDTH; ++x) {
        for (int y = 0; y < HEIGHT; ++y) {
            switch (newtonData[x][y]) {
                case 0:
                    glColor3d(1.0, 0.0, 0.0);  // Red 1st root
                    break;
                case 1:
                    glColor3d(0.0, 1.0, 0.0);  // Green 2nd
                    break;
                case 2:
                    glColor3d(0.0, 0.0, 1.0);  // Blue 3rd
                    break;
                default:
                    glColor3d(0.0, 0.0, 0.0);  // Black - not root points
            }
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
    std::cout << "Threads: " << num_threads << std::endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Newton Pool");
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    computeNewtonFractal();

    glutMainLoop();

    return 0;
}

//g++ -o NewtonPool 4_3.cpp -lGL -lGLU -lglut -fopenmp
