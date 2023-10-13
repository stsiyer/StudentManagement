#ifndef STUDENT_FUNCTIONS
#define STUDENT_FUNCTIONS

#include "common.h"

//================================= Function Prototypes =================================

bool student_operation_handler(int connFD);
int enroll_course(int connFD, int studentId);
int de_enroll_course(int connFD, int studentId);
int view_enrolled_courses(int connFD, int studentID);
bool update_password_student(int connFD, int studentID);

//================================= Function Definition =================================

bool student_operation_handler(int connFD)
{
    int studentID = login_handler(STUDENT, connFD);
    if (studentID)
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000]; // A buffer used for reading & writing to the client
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, STUDENT_LOGIN_SUCCESS);
        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, STUDENT_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing STUDENT_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for STUDENT_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            switch (choice)
            {
            case 1:
                printf("inside case 1");
                enroll_course(connFD, studentID);
                break;
            case 2:
                de_enroll_course(connFD, studentID);
                break;
            case 3:
                view_enrolled_courses(connFD, studentID);
                break;
            case 4:
                update_password_student(connFD, studentID);
                break;
            default:
                writeBytes = write(connFD, STUDENT_LOGOUT, strlen(STUDENT_LOGOUT));
                return false;
            }
        }
    }
    else
    {
        // FACULTY LOGIN FAILED
        return false;
    }
    return true;
}

int enroll_course(int connFD, int studentId)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    Course course;
    sprintf(writeBuffer, "%s\n", STUDENT_ENROLL_COURSE);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing STUDENT_ENROLL_COURSE message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading course name response from client!");
        return false;
    }
    int courseId = atoi(readBuffer);

    course = getCourseById(courseId);

    if (courseId<=0 || courseId>=getNextCourseId() || course.isActive==false)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, STUDENT_ERROR_COURSE_INVALID);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Invalid CourseId message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    Student student = getStudentById(studentId);
    
    // Check if student already enrolled in the course
    for (int i = 0; i < student.noOfCoursesEnrolled; i++)
    {
        if (student.coursesEnrolled[i]==courseId)
        {
            bzero(writeBuffer, sizeof(writeBuffer));
            strcpy(writeBuffer, STUDENT_ERROR_COURSE_TAKEN);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing STUDENT_ERROR_COURSE_TAKEN message to client!");
                return false;
            }
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            return false;
        }
    }
    
    if (course.noEnrolledStudents == course.maxSeats)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, STUDENT_ERROR_COURSE_FULL);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Invalid CourseId message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    course.enrolledStudents[course.noEnrolledStudents++] = studentId;
    student.coursesEnrolled[student.noOfCoursesEnrolled++] = course.id;

    updateCourse(course);
    updateStudent(student);


    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s", STUDENT_ENROLL_COURSE_SUCCESS);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending course enroll success message to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return course.id;

}

int de_enroll_course(int connFD, int studentId)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    Course course;
    sprintf(writeBuffer, "%s\n", STUDENT_UNENROLL_COURSE);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing STUDENT_UNENROLL_COURSE message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading course name response from client!");
        return false;
    }

    int courseId = atoi(readBuffer);
    course = getCourseById(courseId);

    if (courseId<=0 || courseId>=getNextCourseId() || course.isActive==false)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, STUDENT_ERROR_COURSE_INVALID);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Invalid CourseId message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    Student student = getStudentById(studentId);
    bool notFound = true; 
    for (int i = 0; i < course.noEnrolledStudents; i++)
    {
        if (notFound && course.enrolledStudents[i]==studentId)
        {
            notFound = false;
            continue;
        }
        if (notFound==false)
        {
            course.enrolledStudents[i-1] = course.enrolledStudents[i];
        }
    }
    course.noEnrolledStudents--;
    for (int i = 0; i < student.noOfCoursesEnrolled; i++)
    {
        if (student.coursesEnrolled[i] == courseId)
        {
            student.coursesEnrolled[i] = student.coursesEnrolled[student.noOfCoursesEnrolled-1];
            student.noOfCoursesEnrolled--;
        }
    }

    updateCourse(course);
    updateStudent(student);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s", STUDENT_UNENROLL_COURSE_SUCCESS);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending course enroll success message to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return course.id;

}

int view_enrolled_courses(int connFD, int studentID)
{
    size_t writeBytes, readBytes;            // Number of bytes read from / written to the client
    char readBuffer[1000], writeBuffer[1000]; // A buffer used for reading & writing to the client
    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, STUDENT_VIEW_COURSES);
    strcat(writeBuffer, "\n");
    if (writeBytes == -1)
    {
        perror("Error while writing STUDENT_VIEW_COURSES to client!");
        return false;
    }
    Student student = getStudentById(studentID);
    Course course;

    for (int i = 0; i < student.noOfCoursesEnrolled; i++)
    {
        course = getCourseById(student.coursesEnrolled[i]);
        char temp[100];
        sprintf(temp, "CourseID: %d, CourseName: %s\n", course.id, course.name);
        strcat(writeBuffer, temp);
    }
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error while writing Course Details to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return course.id;
    
}

bool update_password_student(int connFD, int studentID)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    writeBytes = write(connFD, ADMIN_MOD_STUDENT_NEW_PASSWORD, strlen(ADMIN_MOD_STUDENT_NEW_PASSWORD));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_STUDENT_NEW_PASSWORD message to client!");
        return false;
    }
    readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting response for student's password from client!");
        return false;
    }
    Student student = getStudentById(studentID);
    char hashedPassword[1000];
    strcpy(hashedPassword, crypt(readBuffer, SALT_BAE));
    printf("\n%s %s\n", readBuffer, hashedPassword);
    strcpy(student.password, hashedPassword);

    if(!updateStudent(student))
    {
        writeBytes = write(connFD, PASSWORD_CHANGE_FAIL, strlen(PASSWORD_CHANGE_FAIL));
        return false;
    }

    writeBytes = write(connFD, ADMIN_MOD_SUCCESS, strlen(ADMIN_MOD_SUCCESS));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_SUCCESS message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
    
}


#endif