#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h> 
#include "../Model/model.h"
#include "../config.h"

int getNextCourseId()
{
    ssize_t readBytes, writeBytes;
    Course previousCourse;
    int CourseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (CourseFileDescriptor == -1 && errno == ENOENT)
    {
        // course file was never created
        return 1;
    }
    else if (CourseFileDescriptor == -1)
    {
        perror("Error while opening Course file");
        return -1;
    }
    else
    {
        int offset = lseek(CourseFileDescriptor, -sizeof(Course), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last Course record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(Course), getpid()};
        int lockingStatus = fcntl(CourseFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on Course record!");
            return false;
        }

        readBytes = read(CourseFileDescriptor, &previousCourse, sizeof(Course));
        if (readBytes == -1)
        {
            perror("Error while reading Course record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(CourseFileDescriptor, F_SETLK, &lock);

        close(CourseFileDescriptor);

        return previousCourse.id + 1;
    }

}

void createCourse(Course newCourse)
{
    ssize_t readBytes, writeBytes;
    int CourseFileDescriptor = open(COURSE_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    if (CourseFileDescriptor == -1)
    {
        perror("Error while creating / opening Course file!");
        return;
    }
    writeBytes = write(CourseFileDescriptor, &newCourse, sizeof(newCourse));
    if (writeBytes == -1)
    {
        perror("Error while writing Course record to file!");
        return;
    }

    close(CourseFileDescriptor);
}

Course getCourseById(int courseID)
{
    Course course;
    course.id = -1;
    ssize_t readBytes;
    off_t offset;
    int lockingStatus;
    int courseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    offset = lseek(courseFileDescriptor, (courseID-1) * sizeof(Course), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required course record!");
        return course;
    }
    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(Course), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on course record!");
        return course;
    }

    readBytes = read(courseFileDescriptor, &course, sizeof(Course));
    if (readBytes == -1)
    {
        perror("Error while reading course record from the file!");
        return course;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(courseFileDescriptor, F_SETLK, &lock);

    close(courseFileDescriptor);

    return course;

}

bool updateCourse(Course course)
{
    ssize_t writeBytes;
    int courseFileDescriptor = open(COURSE_FILE, O_WRONLY);
    if (courseFileDescriptor == -1)
    {
        perror("Error while opening course file");
        return false;
    }
    int offset = lseek(courseFileDescriptor, (course.id-1) * sizeof(Course), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required course record!");
        return false;
    }
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(Course);
    int lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on course record!");
        return false;
    }

    writeBytes = write(courseFileDescriptor, &course, sizeof(Course));
    if (writeBytes == -1)
    {
        perror("Error while writing update course into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(courseFileDescriptor, F_SETLK, &lock);

    close(courseFileDescriptor);
    return true;
}