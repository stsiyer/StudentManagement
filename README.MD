# Academia
## Introduction

Academia Portal for College/University using Socket Programming.
A common platform for Faculty, Student to process Academic curriculum.

## Features

Faculty Registration.
Student Registration.
Course Registration.
Enroll Courses.
Drop Courses.
Deactivate Student.
Change Seat Capacity of Courses.

## Getting Started

There are two C files that needs to be executed.
First execute server.c from one terminal and then go for client.c.
Once both are connected.
We are ready to go.

Roles implemented: Faculty, Student, Admin.

Account holders have to pass through a login system to enter their accounts, and
all these User accounts will be managed by the Administrator.

Server maintains three files as Databases (Course, Faculty and Student) and serves multiple clients concurrently. Clients can connect to the server and access their specific academic details.
To handle concurrent client requests fork system call is used.