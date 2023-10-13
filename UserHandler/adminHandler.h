#ifndef ADMIN_FUNCTIONS
#define ADMIN_FUNCTIONS

#include "common.h"

//================================= Function Prototypes =================================

bool admin_operation_handler(int connFD);
int add_student(int connFD);
int add_faculty(int connFD);
bool modify_student_info(int connFD);
bool modify_faculty_info(int connFD);

//================================= Function Definition =================================

bool admin_operation_handler(int connFD)
{

    if (login_handler(ADMIN, connFD))
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000]; // A buffer used for reading & writing to the client
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ADMIN_LOGIN_SUCCESS);
        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, ADMIN_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing ADMIN_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for ADMIN_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            switch (choice)
            {
            case 1:
                add_student(connFD);
                break;
            case 2:
                add_faculty(connFD);
                break;
            case 3: 
                modify_student_info(connFD);
                break;
            case 4:
                modify_faculty_info(connFD);
                break;
            default:
                writeBytes = write(connFD, ADMIN_LOGOUT, strlen(ADMIN_LOGOUT));
                return false;
            }
        }
    }
    else
    {
        // ADMIN LOGIN FAILED
        return false;
    }
    return true;
}

int add_student(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    Student newStudent;
    sprintf(writeBuffer, "%s\n", ADMIN_ADD_STUDENT_NAME);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_STUDENT_NAME message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading student name response from client!");
        return false;
    }

    strcpy(newStudent.name, readBuffer);

    // EMAIL ID

    sprintf(writeBuffer, "%s\n", ADMIN_ADD_STUDENT_EMAIL);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_STUDENT_EMAIL message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading student email response from client!");
        return false;
    }

    strcpy(newStudent.email, readBuffer);

    // SET DEFAULT PASSWORD
    char hashedPassword[1000];
    strcpy(hashedPassword, crypt(AUTOGEN_PASSWORD, SALT_BAE));
    strcpy(newStudent.password, hashedPassword);

    newStudent.isActive = true;
    newStudent.noOfCoursesEnrolled = 0;
    
    newStudent.id = getNextStudentId();
    // DEFAULT LOGIN ID AS STUDENT NAME
    strcpy(newStudent.login, newStudent.name);
    strcat(newStudent.login, "-");
    sprintf(writeBuffer, "%d", newStudent.id);
    strcat(newStudent.login, writeBuffer);
    createStudent(newStudent);


    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%s-%d\n%s%s", ADMIN_ADD_STUDENT_AUTOGEN_LOGIN, newStudent.name, newStudent.id, ADMIN_ADD_STUDENT_AUTOGEN_PASSWORD, AUTOGEN_PASSWORD);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending student loginID and password to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return newStudent.id;
}

int add_faculty(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    Faculty newFaculty;
    sprintf(writeBuffer, "%s\n", ADMIN_ADD_FACULTY_NAME);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_FACULTY_NAME message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading faculty name response from client!");
        return false;
    }

    strcpy(newFaculty.name, readBuffer);

    // EMAIL ID

    sprintf(writeBuffer, "%s\n", ADMIN_ADD_FACULTY_EMAIL);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_FACULTY_EMAIL message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading faculty name response from client!");
        ;
        return false;
    }

    strcpy(newFaculty.email, readBuffer);

    // SET DEFAULT PASSWORD
    char hashedPassword[1000];
    strcpy(hashedPassword, crypt(AUTOGEN_FACULTY_PASSWORD, SALT_BAE));
    strcpy(newFaculty.password, hashedPassword);

    newFaculty.noOfCoursesOffered = 0;
    
    newFaculty.id = getNextFacultyId();
    // DEFAULT LOGIN ID AS Faculty NAME
    strcpy(newFaculty.login, newFaculty.name);
    strcat(newFaculty.login, "-");
    sprintf(writeBuffer, "%d", newFaculty.id);
    strcat(newFaculty.login, writeBuffer);
    createFaculty(newFaculty);


    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%s-%d\n%s%s", ADMIN_ADD_FACULTY_AUTOGEN_LOGIN, newFaculty.name, newFaculty.id, ADMIN_ADD_FACULTY_AUTOGEN_PASSWORD, AUTOGEN_FACULTY_PASSWORD);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending Faculty loginID and password to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return newFaculty.id;
}

bool modify_student_info(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    Student student;

    int studentID;

    off_t offset;
    int lockingStatus;

    writeBytes = write(connFD, ADMIN_MOD_STUDENT_ID, strlen(ADMIN_MOD_STUDENT_ID));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_STUDENT_ID message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading student ID from client!");
        return false;
    }

    studentID = atoi(readBuffer);

    if (studentID >= getNextStudentId())
    {
        // student File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, STUDENT_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing student_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    if (errno == EINVAL)
    {
        // student record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, STUDENT_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing student_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    student = getStudentById(studentID);
    if(student.id == -1)
    {
        perror("Error while fetching student with given StudentID!");
        return false;
    }
    writeBytes = write(connFD, ADMIN_MOD_STUDENT_MENU, strlen(ADMIN_MOD_STUDENT_MENU));
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

    int choice = atoi(readBuffer);
    
    if (choice == 0)
    { // A non-numeric string was passed to atoi
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    switch (choice)
    {
    case 1:
        writeBytes = write(connFD, ADMIN_MOD_STUDENT_NEW_NAME, strlen(ADMIN_MOD_STUDENT_NEW_NAME));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_STUDENT_NEW_NAME message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for student's new name from client!");
            return false;
        }
        strcpy(student.name, readBuffer);
        break;
    case 2:
        writeBytes = write(connFD, ADMIN_MOD_STUDENT_NEW_EMAIL, strlen(ADMIN_MOD_STUDENT_NEW_EMAIL));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_STUDENT_NEW_EMAIL message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for student's new name from client!");
            return false;
        }
        strcpy(student.email, readBuffer);
        break;
    case 3:
        writeBytes = write(connFD, ADMIN_MOD_STUDENT_NEW_PASSWORD, strlen(ADMIN_MOD_STUDENT_NEW_PASSWORD));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_STUDENT_NEW_PASSWORD message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for student's new name from client!");
            return false;
        }
        char hashedPassword[1000];
        strcpy(hashedPassword, crypt(readBuffer, SALT_BAE));
        strcpy(student.password, hashedPassword);
        break;
    case 4:
        writeBytes = write(connFD, ADMIN_MOD_STUDENT_NEW_STATUS, strlen(ADMIN_MOD_STUDENT_NEW_STATUS));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_STUDENT_NEW_PASSWORD message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for student's new name from client!");
            return false;
        }
        int choice = atoi(readBuffer);
    
        if (choice == 0 || choice>2)
        { 
            // A non-numeric string was passed to atoi
            bzero(writeBuffer, sizeof(writeBuffer));
            strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
                return false;
            }
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            return false;
        }
        student.isActive = (choice == 1) ? true:  false;
        if (choice!=1)
        {
            for (int j = 0; j < student.noOfCoursesEnrolled; j++)
            {
                Course course = getCourseById(student.coursesEnrolled[j]);
                bool notFound = true; 
                for (int i = 0; i < course.noEnrolledStudents; i++)
                {
                    if (notFound && course.enrolledStudents[i]==student.id)
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
                updateCourse(course);
            }
        }       
        break;   
    default:
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, INVALID_MENU_CHOICE);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing INVALID_MENU_CHOICE message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    if(!updateStudent(student))
    {
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

bool modify_faculty_info(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    Faculty faculty;

    int facultyID;

    off_t offset;
    int lockingStatus;

    writeBytes = write(connFD, ADMIN_MOD_FACULTY_ID, strlen(ADMIN_MOD_FACULTY_ID));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_FACULTY_ID message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading faculty ID from client!");
        return false;
    }

    facultyID = atoi(readBuffer);

    if (facultyID >= getNextFacultyId())
    {
        // faculty File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, FACULTY_LOGIN_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing FACULTY_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    if (errno == EINVAL)
    {
        // faculty record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, FACULTY_LOGIN_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing FACULTY_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    faculty = getFacultyById(facultyID);
    if(faculty.id == -1)
    {
        perror("Error while fetching faculty with given facultyID!");
        return false;
    }
    writeBytes = write(connFD, ADMIN_MOD_FACULTY_MENU, strlen(ADMIN_MOD_FACULTY_MENU));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_FACULTY_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting faculty modification menu choice from client!");
        return false;
    }

    int choice = atoi(readBuffer);
    
    if (choice == 0)
    { // A non-numeric string was passed to atoi
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    switch (choice)
    {
    case 1:
        writeBytes = write(connFD, ADMIN_MOD_FACULTY_NEW_NAME, strlen(ADMIN_MOD_FACULTY_NEW_NAME));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_FACULTY_NEW_NAME message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for faculty's new name from client!");
            return false;
        }
        strcpy(faculty.name, readBuffer);
        break;
    case 2:
        writeBytes = write(connFD, ADMIN_MOD_FACULTY_NEW_EMAIL, strlen(ADMIN_MOD_FACULTY_NEW_EMAIL));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_FACULTY_NEW_EMAIL message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for faculty's new name from client!");
            return false;
        }
        strcpy(faculty.email, readBuffer);
        break;
    case 3:
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
        char hashedPassword[1000];
        strcpy(hashedPassword, crypt(readBuffer, SALT_BAE));
        strcpy(faculty.password, hashedPassword);
        break;
    default:
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, INVALID_MENU_CHOICE);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing INVALID_MENU_CHOICE message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    if(!updateFaculty(faculty))
    {
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