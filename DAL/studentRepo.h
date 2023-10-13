#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h> 
#include "../Model/model.h"
#include "../config.h"

int getNextStudentId()
{
    ssize_t readBytes, writeBytes;
    Student previousStudent;
    int studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    if (studentFileDescriptor == -1 && errno == ENOENT)
    {
        // Student file was never created
        return 1;
    }
    else if (studentFileDescriptor == -1)
    {
        perror("Error while opening Student file");
        return -1;
    }
    else
    {
        int offset = lseek(studentFileDescriptor, -sizeof(Student), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last Student record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(Student), getpid()};
        int lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on Student record!");
            return false;
        }

        readBytes = read(studentFileDescriptor, &previousStudent, sizeof(Student));
        if (readBytes == -1)
        {
            perror("Error while reading Student record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(studentFileDescriptor, F_SETLK, &lock);

        close(studentFileDescriptor);

        return previousStudent.id + 1;
    }

}

void createStudent(Student newStudent)
{
    ssize_t readBytes, writeBytes;
    int studentFileDescriptor = open(STUDENT_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    if (studentFileDescriptor == -1)
    {
        perror("Error while creating / opening Student file!");
        return;
    }
    writeBytes = write(studentFileDescriptor, &newStudent, sizeof(newStudent));
    if (writeBytes == -1)
    {
        perror("Error while writing Student record to file!");
        return;
    }

    close(studentFileDescriptor);
}

Student getStudentById(int studentID)
{
    Student student;
    student.id = -1;
    ssize_t readBytes;
    off_t offset;
    int lockingStatus;
    int studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    offset = lseek(studentFileDescriptor, (studentID-1) * sizeof(Student), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required customer record!");
        return student;
    }
    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(Student), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on student record!");
        return student;
    }

    readBytes = read(studentFileDescriptor, &student, sizeof(Student));
    if (readBytes == -1)
    {
        perror("Error while reading student record from the file!");
        return student;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLK, &lock);

    close(studentFileDescriptor);

    return student;

}

bool updateStudent(Student student)
{
    ssize_t writeBytes;
    int studentFileDescriptor = open(STUDENT_FILE, O_WRONLY);
    if (studentFileDescriptor == -1)
    {
        perror("Error while opening student file");
        return false;
    }
    int offset = lseek(studentFileDescriptor, (student.id-1) * sizeof(Student), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(Student);
    int lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on student record!");
        return false;
    }

    writeBytes = write(studentFileDescriptor, &student, sizeof(Student));
    if (writeBytes == -1)
    {
        perror("Error while writing update student info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLKW, &lock);

    close(studentFileDescriptor);
    return true;
}