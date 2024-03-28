#include <iostream>
#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

const int WIDTH = 800;
const int HEIGHT = 800;
const int MAX_ITERATIONS = 1000;
const double SCALE = 0.005;
const double OFFSET_X = -0.5;
const double OFFSET_Y = 0.0;

std::mutex mtx;
std::condition_variable cv;
bool ready = false;
std::queue<int> task_queue;
std::vector<std::vector<double>> mandelbrotData(WIDTH, std::vector<double>(HEIGHT, 0.0));

int mandelbrot(double x0, double y0) {
    double x = 0.0, y = 0.0;
    int iteration = 0;
    while (x*x + y*y <= 4.0 && iteration < MAX_ITERATIONS) {
        double xtemp = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = xtemp;
        ++iteration;
    }
    return iteration;
}

void computeMandelbrot() {
    while (true) {
        int task;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return ready || task_queue.empty(); });
            if (task_queue.empty()) return;
            task = task_queue.front();
            task_queue.pop();
        }
        if (task == -1) break;
        int y = task / WIDTH;
        int x = task % WIDTH;
        double x0 = SCALE * (x - WIDTH / 2) + OFFSET_X;
        double y0 = SCALE * (HEIGHT / 2 - y) + OFFSET_Y;
        int iteration = mandelbrot(x0, y0);
        double brightness = static_cast<double>(MAX_ITERATIONS - iteration) / MAX_ITERATIONS;
        mandelbrotData[x][y] = brightness;
    }
}

void enqueueTasks(int num_threads) {
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            task_queue.push(i * WIDTH + j);
        }
    }
    for (int i = 0; i < num_threads; ++i) {
        task_queue.push(-1);
    }
    ready = true;
    cv.notify_all();
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
    int num_threads = std::thread::hardware_concurrency();
    std::cout << "Number of hardware threads: " << num_threads << std::endl;
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Mandelbrot Set");
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    
    //Сощдание тредов
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(computeMandelbrot);
    }

    enqueueTasks(num_threads);

    //Завершение
    for (auto& thread : threads) {
        thread.join();
    }

    glutMainLoop();
    
    return 0;
}
