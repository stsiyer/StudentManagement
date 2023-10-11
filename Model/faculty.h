#ifndef FACULTY_H
#define FACULTY_H

typedef struct {
    int id;
    char name[20];
    char email[20];
    char login[20];
    char password[50];
    int noOfCoursesOffered;
    int coursesOffered[10];
} Faculty;

#endif
