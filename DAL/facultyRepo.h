#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h> 
#include "../Model/model.h"
#include "../config.h"

int getNextFacultyId()
{
    ssize_t readBytes, writeBytes;
    Faculty previousFaculty;
    int FacultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    if (FacultyFileDescriptor == -1 && errno == ENOENT)
    {
        // Faculty file was never created
        return 1;
    }
    else if (FacultyFileDescriptor == -1)
    {
        perror("Error while opening Faculty file");
        return -1;
    }
    else
    {
        int offset = lseek(FacultyFileDescriptor, -sizeof(Faculty), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last Faculty record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(Faculty), getpid()};
        int lockingStatus = fcntl(FacultyFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on Faculty record!");
            return false;
        }

        readBytes = read(FacultyFileDescriptor, &previousFaculty, sizeof(Faculty));
        if (readBytes == -1)
        {
            perror("Error while reading Faculty record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(FacultyFileDescriptor, F_SETLK, &lock);

        close(FacultyFileDescriptor);

        return previousFaculty.id + 1;
    }

}

void createFaculty(Faculty newFaculty)
{
    ssize_t readBytes, writeBytes;
    int FacultyFileDescriptor = open(FACULTY_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    if (FacultyFileDescriptor == -1)
    {
        perror("Error while creating / opening Faculty file!");
        return;
    }
    writeBytes = write(FacultyFileDescriptor, &newFaculty, sizeof(newFaculty));
    if (writeBytes == -1)
    {
        perror("Error while writing Faculty record to file!");
        return;
    }

    close(FacultyFileDescriptor);
}

Faculty getFacultyById(int facultyID)
{
    Faculty faculty;
    faculty.id = -1;
    ssize_t readBytes;
    off_t offset;
    int lockingStatus;
    int facultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    offset = lseek(facultyFileDescriptor, (facultyID-1) * sizeof(Faculty), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required faculty record!");
        return faculty;
    }
    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(Faculty), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on faculty record!");
        return faculty;
    }

    readBytes = read(facultyFileDescriptor, &faculty, sizeof(Faculty));
    if (readBytes == -1)
    {
        perror("Error while reading faculty record from the file!");
        return faculty;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLK, &lock);

    close(facultyFileDescriptor);

    return faculty;

}

bool updateFaculty(Faculty faculty)
{
    ssize_t writeBytes;
    int facultyFileDescriptor = open(FACULTY_FILE, O_WRONLY);
    if (facultyFileDescriptor == -1)
    {
        perror("Error while opening faculty file");
        return false;
    }
    int offset = lseek(facultyFileDescriptor, (faculty.id-1) * sizeof(Faculty), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required faculty record!");
        return false;
    }
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    int lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on faculty record!");
        return false;
    }

    writeBytes = write(facultyFileDescriptor, &faculty, sizeof(Faculty));
    if (writeBytes == -1)
    {
        perror("Error while writing update faculty info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLKW, &lock);

    close(facultyFileDescriptor);
    return true;
}