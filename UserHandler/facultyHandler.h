#ifndef FACULTY_FUNCTIONS
#define FACULTY_FUNCTIONS

#include "common.h"

//================================= Function Prototypes =================================

bool faculty_operation_handler(int connFD);
int add_course(int connFD, int facultyID);
bool update_password_faculty(int connFD, int facultyID);
bool view_course_enrollments(int connFD, int facultyID);
bool remove_offered_course(int connFD, int facultyID);
void remove_course_from_student(int studentId, int courseId);
bool update_course_capacity(int connFD, int facultyID);

//================================= Function Definition =================================

bool faculty_operation_handler(int connFD)
{
    int facultyID = login_handler(FACULTY, connFD);
    if (facultyID)
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000]; // A buffer used for reading & writing to the client
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, FACULTY_LOGIN_SUCCESS);
        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, FACULTY_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing FACULTY_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for FACULTY_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            switch (choice)
            {
            case 1:
                add_course(connFD, facultyID);
                break;
            case 2:
                remove_offered_course(connFD, facultyID);
                break;
            case 3:
                view_course_enrollments(connFD, facultyID);
                break;
            case 4:
                update_password_faculty(connFD, facultyID);
                break;
            case 5:
                update_course_capacity(connFD, facultyID);
                break;
            default:
                writeBytes = write(connFD, FACULTY_LOGOUT, strlen(FACULTY_LOGOUT));
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

int add_course(int connFD, int facultyID)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    Course newCourse;
    sprintf(writeBuffer, "%s\n", FACULTY_ADD_COURSE_NAME);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing FACULTY_ADD_COURSE_NAME message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading course name response from client!");
        return false;
    }

    strcpy(newCourse.name, readBuffer);

    sprintf(writeBuffer, "%s\n", FACULTY_ADD_COURSE_LIMIT);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing FACULTY_ADD_COURSE_LIMIT message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading course name response from client!");
        return false;
    }

    int limit = atoi(readBuffer);
    if (limit>COURSE_MAX_SEATS)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, FACULTY_ERROR_COURSE_LIMIT);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Invalid CourseId message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    newCourse.maxSeats = limit;
    newCourse.noEnrolledStudents = 0;
    newCourse.facultyId = facultyID;
    newCourse.id = getNextCourseId();
    newCourse.isActive = true;

    Faculty faculty = getFacultyById(facultyID);
    faculty.coursesOffered[faculty.noOfCoursesOffered++] = newCourse.id;

    createCourse(newCourse);
    updateFaculty(faculty);


    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s", FACULTY_ADD_COURSE_SUCCESS);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending course success message to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return newCourse.id;
}

bool update_password_faculty(int connFD, int facultyID)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    writeBytes = write(connFD, ADMIN_MOD_FACULTY_NEW_PASSWORD, strlen(ADMIN_MOD_FACULTY_NEW_PASSWORD));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_FACULTY_NEW_PASSWORD message to client!");
        return false;
    }
    readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting response for faculty's new name from client!");
        return false;
    }
    Faculty faculty = getFacultyById(facultyID);
    char hashedPassword[1000];
    strcpy(hashedPassword, crypt(readBuffer, SALT_BAE));
    strcpy(faculty.password, hashedPassword);

    updateFaculty(faculty);

    if(!updateFaculty(faculty))
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

bool view_course_enrollments(int connFD, int facultyID)
{
    size_t writeBytes, readBytes;            // Number of bytes read from / written to the client
    char writeBuffer[1000], readBuffer[1000]; // A buffer used for reading & writing to the client
    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, FACULTY_VIEW_COURSE_ENROLLENT);
    strcat(writeBuffer, "\n\n");
    if (writeBytes == -1)
    {
        perror("Error while writing STUDENT_VIEW_COURSES to client!");
        return false;
    }
    Faculty faculty = getFacultyById(facultyID);
    for (int i = 0; i < faculty.noOfCoursesOffered; i++)
    {
        Course course = getCourseById(faculty.coursesOffered[i]);
        if (course.isActive==false)
        {
            continue;
        }
        char courseDetails[200];
        sprintf(courseDetails, "\tFor Course: %s\n", course.name);
        strcat(writeBuffer, courseDetails);
        for (int i = 0; i < course.noEnrolledStudents; i++)
        {
            Student student = getStudentById(course.enrolledStudents[i]);
            if (student.isActive==false)
            {
                continue;
            }
            char studentDetails[200];
            sprintf(studentDetails, "StudentID: %d, StudentName: %s\n", student.id, student.name);
            printf("StudentID: %d, StudentName: %s\n", student.id, student.name);
            strcat(writeBuffer, studentDetails);
        }
        strcat(writeBuffer, "\n");           

    }
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error while writing Course Enrollments to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
    
}

bool remove_offered_course(int connFD, int facultyID)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    Course course;
    sprintf(writeBuffer, "%s\n", FACULTY_REMOVE_COURSE);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing FACULTY_REMOVE_COURSE message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading course ID response from client!");
        return false;
    }
    int courseId = atoi(readBuffer);

    Faculty faculty = getFacultyById(facultyID);
    int index = -1;
    for (int i = 0; i < faculty.noOfCoursesOffered; i++)
    {
        if (faculty.coursesOffered[i]==courseId)
        {
            index = i;
            break;
        }
    }
    if (index==-1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, FACULTY_ERROR_COURSE_INVALID);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing FACULTY_ERROR_COURSE_INVALID message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    faculty.coursesOffered[index] = faculty.coursesOffered[--faculty.noOfCoursesOffered];
    course = getCourseById(courseId);

    for (int i = 0; i < course.noEnrolledStudents; i++)
    {
        remove_course_from_student(course.enrolledStudents[i], courseId);
        
    }
    course.isActive = false;
    course.noEnrolledStudents = 0;

    updateCourse(course);
    updateFaculty(faculty);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s", FACULTY_REMOVE_COURSE_SUCCESS);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending course enroll success message to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
    
}

void remove_course_from_student(int studentId, int courseId)
{
    Student student = getStudentById(studentId);
    for (int i = 0; i < student.noOfCoursesEnrolled; i++)
    {
        if (student.coursesEnrolled[i] == courseId)
        {
            student.coursesEnrolled[i] = student.coursesEnrolled[--student.noOfCoursesEnrolled];
            break;
        }
    }
    updateStudent(student);
    return;
}

bool update_course_capacity(int connFD, int facultyID)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    Course course;
    sprintf(writeBuffer, "%s\n", FACULTY_UPDATE_COURSE);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing FACULTY_UPDATE_COURSE message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading course ID response from client!");
        return false;
    }
    int courseId = atoi(readBuffer);

    Faculty faculty = getFacultyById(facultyID);
    int index = -1;
    for (int i = 0; i < faculty.noOfCoursesOffered; i++)
    {
        if (faculty.coursesOffered[i]==courseId)
        {
            index = i;
            break;
        }
    }
    if (index==-1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, FACULTY_ERROR_COURSE_INVALID);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing FACULTY_ERROR_COURSE_INVALID message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    writeBytes = write(connFD, FACULTY_UPDATE_COURSE_CAPACITY, strlen(FACULTY_UPDATE_COURSE_CAPACITY));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_student_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting student modification menu choice from client!");
        return false;
    }

    int capacity = atoi(readBuffer);
    if (capacity > COURSE_MAX_SEATS || capacity<1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, FACULTY_ERROR_COURSE_INVALID);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing FACULTY_ERROR_COURSE_INVALID message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    course = getCourseById(courseId);
    if (capacity < course.noEnrolledStudents)
    {
        for (int i = capacity; i < course.noEnrolledStudents; i++)
        {
            remove_course_from_student(course.enrolledStudents[i] ,courseId);
        }
        course.noEnrolledStudents = capacity;
    }
    course.maxSeats = capacity;
    updateCourse(course);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s", FACULTY_UPDATE_COURSE_SUCCESS);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending FACULTY_UPDATE_COURSE_SUCCESS message to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read


    return true;
}

#endif
