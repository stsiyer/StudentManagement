#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include "Model/model.h"

const char* courseFilePath = "course.txt";

sem_t* semaphore;

void initializeSem() {
    semaphore = sem_open("course_semaphore1", O_CREAT, 0644, 1);
    if (semaphore == SEM_FAILED) {
        perror("Failed to initialize semaphore");
        exit(EXIT_FAILURE);
    }
}

void createCourse(const Course course) {
    sem_wait(semaphore);

    int fd = open(courseFilePath, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }
    
    // Write the new student to the file
    write(fd, &course, sizeof(Course));
    close(fd);

    sem_post(semaphore);
}

Course getCourseById(int courseId) {
    sem_wait(semaphore);

    int dataFileDescriptor = open(courseFilePath, O_RDONLY);
    Course tempCourse;
    Course result = {0};

    while (read(dataFileDescriptor, &tempCourse, sizeof(Course)) > 0) {
        if (tempCourse.id == courseId) {
            result = tempCourse;
            break;
        }
    }

    close(dataFileDescriptor);

    sem_post(semaphore);

    return result;
}

void showAllCourses() {
    sem_wait(semaphore);

    int fd = open(courseFilePath, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }
    
    Course course;
    while (read(fd, &course, sizeof(Course)) > 0) {
        printf("ID: %d\nName: %s\nFacultyId: %d\nEnrolledStudents: %d\n", course.id, course.name, course.facultyId, course.enrolledStudents);
        // Display courses enrolled here
    }
    close(fd);

    sem_post(semaphore);
}

void updateCourse(const Course updateCourse) {
    sem_wait(semaphore);

    int fd = open(courseFilePath, O_RDWR);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }
    
    Course course;
    int found = 0;
    while (read(fd, &course, sizeof(Course)) > 0) {
        if (course.id == updateCourse.id) {
            // Update student information
            lseek(fd, -sizeof(Course), SEEK_CUR);
            write(fd, &updateCourse, sizeof(Course));
            found = 1;
            break;
        }
    }
    close(fd);
    
    if (!found) {
        printf("Course with ID %d not found.\n", updateCourse.id);
    }

    sem_post(semaphore);
}


int main() {
    initializeSem();

    Course course1 = {1, "Math", 101, 30, 50};
    Course course2 = {2, "History", 102, 25, 40};

    createCourse(course1);
    createCourse(course2);

    showAllCourses();
    printf("\n");

    Course retrievedCourse = getCourseById(2);
    printf("\nRetrieved Course: ID: %d, Name: %s\n", retrievedCourse.id, retrievedCourse.name);

    course2.facultyId = 100;
    updateCourse(course2);
    printf("\n\nAfter Update\n\n");
    showAllCourses();

    sem_close(semaphore);
    sem_unlink("course_semaphore");

    return 0;
}
