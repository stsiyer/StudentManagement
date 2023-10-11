#ifndef STUDENT_H
#define STUDENT_H

typedef struct {
    int id;
    char name[20];
    char email[20];
    char login[20];
    char password[50];
    bool isActive;
    int noOfCoursesEnrolled;
    int coursesEnrolled[10];
} Student;

#endif
