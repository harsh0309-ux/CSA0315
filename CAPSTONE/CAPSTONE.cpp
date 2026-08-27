#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CREDITS 20

/* =========================================================
   STATUS
   ========================================================= */

typedef enum {
    PENDING,
    APPROVED,
    REJECTED,
    WAITLISTED,
    COMPLETED
} Status;


/* =========================================================
   STUDENT LINKED LIST
   ========================================================= */

typedef struct Student {
    int id;
    char name[50];
    char email[60];
    char password[30];

    struct Student *next;
} Student;


/* =========================================================
   FACULTY LINKED LIST
   ========================================================= */

typedef struct Faculty {
    int id;
    char name[50];
    char email[60];
    char password[30];

    struct Faculty *next;
} Faculty;


/* =========================================================
   COURSE LINKED LIST
   MAIN LINKED LIST FOR COURSE MANAGEMENT
   ========================================================= */

typedef struct Course {
    int id;
    char name[80];

    int credits;
    int capacity;

    int facultyID;

    /*
       prerequisiteID = 0 means
       the course has no prerequisite.
    */
    int prerequisiteID;

    struct Course *next;
} Course;


/* =========================================================
   APPLICATION LINKED LIST
   ========================================================= */

typedef struct Application {
    int applicationID;

    int studentID;
    int courseID;

    Status status;

    struct Application *next;
} Application;


/* =========================================================
   FIFO WAITLIST QUEUE
   ========================================================= */

typedef struct WaitNode {
    int applicationID;
    int studentID;
    int courseID;

    struct WaitNode *next;
} WaitNode;


typedef struct Queue {
    WaitNode *front;
    WaitNode *rear;
} Queue;


/* =========================================================
   GLOBAL HEAD POINTERS
   ========================================================= */

Student *studentHead = NULL;

Faculty *facultyHead = NULL;

Course *courseHead = NULL;

Application *applicationHead = NULL;


/* =========================================================
   GLOBAL APPLICATION ID
   ========================================================= */

int nextApplicationID = 1001;


/* =========================================================
   FUNCTION DECLARATIONS
   ========================================================= */

/* Student */
void studentRegister();
Student* studentLogin();
void studentMenu(Student *student);

/* Faculty */
void facultyRegister();
Faculty* facultyLogin();
void facultyMenu(Faculty *faculty);

/* Admin */
void adminLogin();
void adminMenu();

/* Course */
void addCourse();
void displayCourses();
void searchCourse();
void modifyCourse();
void deleteCourse();

Course* findCourse(int id);

/* Student course operations */
void applyCourse(Student *student);
void viewStudentApplications(Student *student);
void viewApprovedCourses(Student *student);
void viewStudentWaitlistPositions(Student *student);

/* Eligibility */
int hasCompletedPrerequisite(Student *student, Course *course);
int getStudentCredits(Student *student);
int checkEligibility(Student *student, Course *course);

/* Faculty */
void viewApplications(Faculty *faculty);
void approveApplication(Faculty *faculty);
void rejectApplication(Faculty *faculty);
void viewEnrolledStudents(Faculty *faculty);
void removeStudentFromCourse(Faculty *faculty);
void markCourseCompleted(Faculty *faculty);
void viewCourseWaitlist(Faculty *faculty);

/* Waitlist */
void initializeQueue(Queue *q);
void enqueue(Queue *q,
             int applicationID,
             int studentID,
             int courseID);

WaitNode* dequeue(Queue *q);

void buildWaitlist(int courseID, Queue *q);

void promoteWaitlistedStudent(Course *course);

/* Search */
Student* findStudent(int id);
Faculty* findFaculty(int id);
Application* findApplication(int id);

/* Utility */
const char* statusString(Status status);

int getEnrolledCount(int courseID);

int getWaitlistPosition(int studentID, int courseID);

/* Display */
void displayAllStudents();
void displayAllFaculty();
void displayAllApplications();
void displayAllWaitlists();

/* File handling */
void saveStudents();
void saveFaculty();
void saveCourses();
void saveApplications();

void loadStudents();
void loadFaculty();
void loadCourses();
void loadApplications();

void saveAll();
void loadAll();


/* =========================================================
   MAIN
   ========================================================= */

int main()
{
    int choice;

    loadAll();

    do
    {
        printf("\n\n");
        printf("====================================================\n");
        printf("     ONLINE COURSE REGISTRATION AND MANAGEMENT\n");
        printf("====================================================\n");

        printf("\n1. Student");
        printf("\n2. Faculty");
        printf("\n3. Admin");
        printf("\n4. Exit");

        printf("\n\nEnter your choice: ");
        scanf("%d", &choice);

        switch(choice)
        {
            case 1:
            {
                int ch;

                printf("\n========== STUDENT ==========\n");

                printf("1. Register\n");
                printf("2. Login\n");
                printf("3. Back\n");

                printf("\nEnter choice: ");
                scanf("%d", &ch);

                if(ch == 1)
                {
                    studentRegister();
                }
                else if(ch == 2)
                {
                    Student *student = studentLogin();

                    if(student != NULL)
                    {
                        studentMenu(student);
                    }
                }

                break;
            }

            case 2:
            {
                int ch;

                printf("\n========== FACULTY ==========\n");

                printf("1. Register\n");
                printf("2. Login\n");
                printf("3. Back\n");

                printf("\nEnter choice: ");
                scanf("%d", &ch);

                if(ch == 1)
                {
                    facultyRegister();
                }
                else if(ch == 2)
                {
                    Faculty *faculty = facultyLogin();

                    if(faculty != NULL)
                    {
                        facultyMenu(faculty);
                    }
                }

                break;
            }

            case 3:
                adminLogin();
                break;

            case 4:
                saveAll();

                printf("\nAll data saved successfully.\n");
                printf("Thank you for using the system!\n");

                break;

            default:
                printf("\nInvalid choice!\n");
        }

    } while(choice != 4);

    return 0;
}


/* =========================================================
   STUDENT REGISTRATION
   ========================================================= */

void studentRegister()
{
    Student *newStudent;

    newStudent = (Student*)malloc(sizeof(Student));

    if(newStudent == NULL)
    {
        printf("\nMemory allocation failed!\n");
        return;
    }

    printf("\n========== STUDENT REGISTRATION ==========\n");

    printf("Enter Student ID: ");
    scanf("%d", &newStudent->id);

    if(findStudent(newStudent->id) != NULL)
    {
        printf("\nStudent ID already exists!\n");

        free(newStudent);
        return;
    }

    printf("Enter Name: ");
    scanf(" %[^\n]", newStudent->name);

    printf("Enter Email: ");
    scanf("%s", newStudent->email);

    printf("Enter Password: ");
    scanf("%s", newStudent->password);

    newStudent->next = NULL;

    if(studentHead == NULL)
    {
        studentHead = newStudent;
    }
    else
    {
        Student *temp = studentHead;

        while(temp->next != NULL)
        {
            temp = temp->next;
        }

        temp->next = newStudent;
    }

    saveStudents();

    printf("\nStudent registered successfully!\n");
}


/* =========================================================
   STUDENT LOGIN
   ========================================================= */

Student* studentLogin()
{
    int id;
    char password[30];

    printf("\n========== STUDENT LOGIN ==========\n");

    printf("Enter Student ID: ");
    scanf("%d", &id);

    printf("Enter Password: ");
    scanf("%s", password);

    Student *student = findStudent(id);

    if(student != NULL &&
       strcmp(student->password, password) == 0)
    {
        printf("\nLogin successful!\n");
        printf("Welcome, %s!\n", student->name);

        return student;
    }

    printf("\nInvalid Student ID or Password!\n");

    return NULL;
}


/* =========================================================
   STUDENT MENU
   ========================================================= */

void studentMenu(Student *student)
{
    int choice;

    do
    {
        printf("\n\n");
        printf("====================================\n");
        printf("         STUDENT DASHBOARD\n");
        printf("====================================\n");

        printf("Current Credits : %d / %d\n",
               getStudentCredits(student),
               MAX_CREDITS);

        printf("\n1. View Available Courses");
        printf("\n2. Search Course");
        printf("\n3. Apply for Course");
        printf("\n4. View My Applications");
        printf("\n5. View My Approved/Completed Courses");
        printf("\n6. View My Waitlist Positions");
        printf("\n7. Logout");

        printf("\n\nEnter choice: ");
        scanf("%d", &choice);

        switch(choice)
        {
            case 1:
                displayCourses();
                break;

            case 2:
                searchCourse();
                break;

            case 3:
                applyCourse(student);
                break;

            case 4:
                viewStudentApplications(student);
                break;

            case 5:
                viewApprovedCourses(student);
                break;

            case 6:
                viewStudentWaitlistPositions(student);
                break;

            case 7:
                printf("\nLogged out successfully.\n");
                break;

            default:
                printf("\nInvalid choice!\n");
        }

    } while(choice != 7);
}


/* =========================================================
   FACULTY REGISTRATION
   ========================================================= */

void facultyRegister()
{
    Faculty *newFaculty;

    newFaculty = (Faculty*)malloc(sizeof(Faculty));

    if(newFaculty == NULL)
    {
        printf("\nMemory allocation failed!\n");
        return;
    }

    printf("\n========== FACULTY REGISTRATION ==========\n");

    printf("Enter Faculty ID: ");
    scanf("%d", &newFaculty->id);

    if(findFaculty(newFaculty->id) != NULL)
    {
        printf("\nFaculty ID already exists!\n");

        free(newFaculty);
        return;
    }

    printf("Enter Name: ");
    scanf(" %[^\n]", newFaculty->name);

    printf("Enter Email: ");
    scanf("%s", newFaculty->email);

    printf("Enter Password: ");
    scanf("%s", newFaculty->password);

    newFaculty->next = NULL;

    if(facultyHead == NULL)
    {
        facultyHead = newFaculty;
    }
    else
    {
        Faculty *temp = facultyHead;

        while(temp->next != NULL)
        {
            temp = temp->next;
        }

        temp->next = newFaculty;
    }

    saveFaculty();

    printf("\nFaculty registered successfully!\n");
}


/* =========================================================
   FACULTY LOGIN
   ========================================================= */

Faculty* facultyLogin()
{
    int id;
    char password[30];

    printf("\n========== FACULTY LOGIN ==========\n");

    printf("Enter Faculty ID: ");
    scanf("%d", &id);

    printf("Enter Password: ");
    scanf("%s", password);

    Faculty *faculty = findFaculty(id);

    if(faculty != NULL &&
       strcmp(faculty->password, password) == 0)
    {
        printf("\nLogin successful!\n");
        printf("Welcome, %s!\n", faculty->name);

        return faculty;
    }

    printf("\nInvalid Faculty ID or Password!\n");

    return NULL;
}


/* =========================================================
   FACULTY MENU
   ========================================================= */

void facultyMenu(Faculty *faculty)
{
    int choice;

    do
    {
        printf("\n\n");
        printf("====================================\n");
        printf("         FACULTY DASHBOARD\n");
        printf("====================================\n");

        printf("\n1. View My Courses");
        printf("\n2. View Student Applications");
        printf("\n3. Approve Student");
        printf("\n4. Reject Student");
        printf("\n5. View Enrolled Students");
        printf("\n6. View Course Waitlist");
        printf("\n7. Mark Student Course Completed");
        printf("\n8. Logout");

        printf("\n\nEnter choice: ");
        scanf("%d", &choice);

        switch(choice)
        {
            case 1:
                displayCourses();
                break;

            case 2:
                viewApplications(faculty);
                break;

            case 3:
                approveApplication(faculty);
                break;

            case 4:
                rejectApplication(faculty);
                break;

            case 5:
                viewEnrolledStudents(faculty);
                break;

            case 6:
                viewCourseWaitlist(faculty);
                break;

            case 7:
                markCourseCompleted(faculty);
                break;

            case 8:
                printf("\nLogged out successfully.\n");
                break;

            default:
                printf("\nInvalid choice!\n");
        }

    } while(choice != 8);
}


/* =========================================================
   ADMIN LOGIN
   ========================================================= */

void adminLogin()
{
    char username[30];
    char password[30];

    printf("\n========== ADMIN LOGIN ==========\n");

    printf("Username: ");
    scanf("%s", username);

    printf("Password: ");
    scanf("%s", password);

    if(strcmp(username, "admin") == 0 &&
       strcmp(password, "admin123") == 0)
    {
        printf("\nAdmin login successful!\n");

        adminMenu();
    }
    else
    {
        printf("\nInvalid Admin Login!\n");
    }
}


/* =========================================================
   ADMIN MENU
   ========================================================= */

void adminMenu()
{
    int choice;

    do
    {
        printf("\n\n");
        printf("====================================\n");
        printf("          ADMIN DASHBOARD\n");
        printf("====================================\n");

        printf("\n1. Add Course");
        printf("\n2. Delete Course");
        printf("\n3. Modify Course");
        printf("\n4. View All Courses");
        printf("\n5. Search Course");
        printf("\n6. View All Students");
        printf("\n7. View All Faculty");
        printf("\n8. View All Applications");
        printf("\n9. View All Waitlists");
        printf("\n10. Logout");

        printf("\n\nEnter choice: ");
        scanf("%d", &choice);

        switch(choice)
        {
            case 1:
                addCourse();
                break;

            case 2:
                deleteCourse();
                break;

            case 3:
                modifyCourse();
                break;

            case 4:
                displayCourses();
                break;

            case 5:
                searchCourse();
                break;

            case 6:
                displayAllStudents();
                break;

            case 7:
                displayAllFaculty();
                break;

            case 8:
                displayAllApplications();
                break;

            case 9:
                displayAllWaitlists();
                break;

            case 10:
                printf("\nLogged out successfully.\n");
                break;

            default:
                printf("\nInvalid choice!\n");
        }

    } while(choice != 10);
}


/* =========================================================
   ADD COURSE
   LINKED LIST INSERTION
   ========================================================= */

void addCourse()
{
    Course *newCourse;

    newCourse = (Course*)malloc(sizeof(Course));

    if(newCourse == NULL)
    {
        printf("\nMemory allocation failed!\n");
        return;
    }

    printf("\n========== ADD COURSE ==========\n");

    printf("Enter Course ID: ");
    scanf("%d", &newCourse->id);

    if(findCourse(newCourse->id) != NULL)
    {
        printf("\nCourse ID already exists!\n");

        free(newCourse);
        return;
    }

    printf("Enter Course Name: ");
    scanf(" %[^\n]", newCourse->name);

    printf("Enter Course Credits: ");
    scanf("%d", &newCourse->credits);

    if(newCourse->credits <= 0)
    {
        printf("\nCredits must be greater than 0.\n");

        free(newCourse);
        return;
    }

    printf("Enter Course Capacity: ");
    scanf("%d", &newCourse->capacity);

    if(newCourse->capacity <= 0)
    {
        printf("\nCapacity must be greater than 0.\n");

        free(newCourse);
        return;
    }

    printf("Enter Faculty ID: ");
    scanf("%d", &newCourse->facultyID);

    if(findFaculty(newCourse->facultyID) == NULL)
    {
        printf("\nFaculty does not exist!\n");

        free(newCourse);
        return;
    }

    printf("\nEnter Prerequisite Course ID.");
    printf("\nEnter 0 if there is no prerequisite.\n");

    printf("Prerequisite Course ID: ");
    scanf("%d", &newCourse->prerequisiteID);

    if(newCourse->prerequisiteID != 0)
    {
        if(findCourse(newCourse->prerequisiteID) == NULL)
        {
            printf("\nPrerequisite course does not exist!\n");

            free(newCourse);
            return;
        }

        if(newCourse->prerequisiteID == newCourse->id)
        {
            printf("\nA course cannot be its own prerequisite!\n");

            free(newCourse);
            return;
        }
    }

    newCourse->next = NULL;

    /*
       LINKED LIST INSERTION
    */

    if(courseHead == NULL)
    {
        courseHead = newCourse;
    }
    else
    {
        Course *temp = courseHead;

        while(temp->next != NULL)
        {
            temp = temp->next;
        }

        temp->next = newCourse;
    }

    saveCourses();

    printf("\nCourse added successfully!\n");
}


/* =========================================================
   DISPLAY COURSES
   ========================================================= */

void displayCourses()
{
    Course *temp = courseHead;

    if(temp == NULL)
    {
        printf("\nNo courses available.\n");
        return;
    }

    printf("\n========== COURSE CATALOG ==========\n");

    while(temp != NULL)
    {
        int enrolled = getEnrolledCount(temp->id);

        Faculty *faculty =
            findFaculty(temp->facultyID);

        printf("\nCourse ID       : %d", temp->id);
        printf("\nCourse Name     : %s", temp->name);
        printf("\nCredits         : %d", temp->credits);
        printf("\nCapacity        : %d", temp->capacity);
        printf("\nEnrolled        : %d", enrolled);
        printf("\nAvailable Seats : %d",
               temp->capacity - enrolled);

        if(faculty != NULL)
        {
            printf("\nFaculty         : %s",
                   faculty->name);
        }

        if(temp->prerequisiteID == 0)
        {
            printf("\nPrerequisite    : None");
        }
        else
        {
            Course *pre =
                findCourse(temp->prerequisiteID);

            if(pre != NULL)
            {
                printf("\nPrerequisite    : %s (%d)",
                       pre->name,
                       pre->id);
            }
        }

        if(enrolled >= temp->capacity)
        {
            printf("\nStatus          : FULL");
        }
        else
        {
            printf("\nStatus          : SEATS AVAILABLE");
        }

        printf("\n----------------------------------------\n");

        temp = temp->next;
    }
}


/* =========================================================
   SEARCH COURSE
   BY ID OR NAME
   ========================================================= */

void searchCourse()
{
    int choice;

    printf("\n========== SEARCH COURSE ==========\n");

    printf("1. Search by Course ID\n");
    printf("2. Search by Course Name\n");

    printf("\nEnter choice: ");
    scanf("%d", &choice);

    if(choice == 1)
    {
        int id;

        printf("\nEnter Course ID: ");
        scanf("%d", &id);

        Course *course = findCourse(id);

        if(course == NULL)
        {
            printf("\nCourse not found!\n");
            return;
        }

        printf("\nCourse ID       : %d", course->id);
        printf("\nCourse Name     : %s", course->name);
        printf("\nCredits         : %d", course->credits);
        printf("\nCapacity        : %d", course->capacity);
        printf("\nEnrolled        : %d",
               getEnrolledCount(course->id));
        printf("\nAvailable Seats : %d",
               course->capacity -
               getEnrolledCount(course->id));

        Faculty *faculty =
            findFaculty(course->facultyID);

        if(faculty != NULL)
        {
            printf("\nFaculty         : %s",
                   faculty->name);
        }

        if(course->prerequisiteID == 0)
        {
            printf("\nPrerequisite    : None");
        }
        else
        {
            Course *pre =
                findCourse(course->prerequisiteID);

            if(pre != NULL)
            {
                printf("\nPrerequisite    : %s (%d)",
                       pre->name,
                       pre->id);
            }
        }

        printf("\n");
    }
    else if(choice == 2)
    {
        char name[80];

        printf("\nEnter Course Name: ");
        scanf(" %[^\n]", name);

        Course *temp = courseHead;

        int found = 0;

        while(temp != NULL)
        {
            if(strstr(temp->name, name) != NULL)
            {
                printf("\nCourse ID       : %d",
                       temp->id);

                printf("\nCourse Name     : %s",
                       temp->name);

                printf("\nCredits         : %d",
                       temp->credits);

                printf("\nCapacity        : %d",
                       temp->capacity);

                printf("\nEnrolled        : %d",
                       getEnrolledCount(temp->id));

                printf("\nAvailable Seats : %d",
                       temp->capacity -
                       getEnrolledCount(temp->id));

                printf("\n-------------------------------\n");

                found = 1;
            }

            temp = temp->next;
        }

        if(!found)
        {
            printf("\nCourse not found!\n");
        }
    }
    else
    {
        printf("\nInvalid choice!\n");
    }
}


/* =========================================================
   MODIFY COURSE
   ========================================================= */

void modifyCourse()
{
    int id;

    printf("\nEnter Course ID to modify: ");
    scanf("%d", &id);

    Course *course = findCourse(id);

    if(course == NULL)
    {
        printf("\nCourse not found!\n");
        return;
    }

    int choice;

    do
    {
        printf("\n========== MODIFY COURSE ==========\n");

        printf("1. Modify Course Name\n");
        printf("2. Modify Credits\n");
        printf("3. Modify Capacity\n");
        printf("4. Modify Faculty\n");
        printf("5. Modify Prerequisite\n");
        printf("6. Back\n");

        printf("\nEnter choice: ");
        scanf("%d", &choice);

        switch(choice)
        {
            case 1:
                printf("\nEnter new Course Name: ");
                scanf(" %[^\n]", course->name);
                break;

            case 2:
                printf("\nEnter new Credits: ");
                scanf("%d", &course->credits);

                if(course->credits <= 0)
                {
                    printf("\nInvalid credits.\n");
                }
                break;

            case 3:
                printf("\nEnter new Capacity: ");
                scanf("%d", &course->capacity);

                if(course->capacity <= 0)
                {
                    printf("\nInvalid capacity.\n");
                }
                else
                {
                    /*
                       If capacity increases,
                       automatically promote waitlisted students.
                    */

                    while(getEnrolledCount(course->id)
                          < course->capacity)
                    {
                        Queue q;

                        initializeQueue(&q);

                        buildWaitlist(course->id, &q);

                        WaitNode *node =
                            dequeue(&q);

                        if(node == NULL)
                        {
                            break;
                        }

                        Application *app =
                            findApplication(
                                node->applicationID);

                        if(app != NULL)
                        {
                            app->status = APPROVED;

                            printf("\nWaitlisted Student %d "
                                   "promoted to APPROVED.\n",
                                   app->studentID);
                        }

                        free(node);
                    }

                    saveApplications();
                }

                break;

            case 4:
            {
                int facultyID;

                printf("\nEnter new Faculty ID: ");
                scanf("%d", &facultyID);

                if(findFaculty(facultyID) == NULL)
                {
                    printf("\nFaculty does not exist!\n");
                }
                else
                {
                    course->facultyID = facultyID;
                }

                break;
            }

            case 5:
            {
                int preID;

                printf("\nEnter new Prerequisite Course ID: ");
                printf("\nEnter 0 for no prerequisite: ");

                scanf("%d", &preID);

                if(preID != 0 &&
                   findCourse(preID) == NULL)
                {
                    printf("\nPrerequisite course not found!\n");
                }
                else if(preID == course->id)
                {
                    printf("\nA course cannot be its own prerequisite!\n");
                }
                else
                {
                    course->prerequisiteID = preID;
                }

                break;
            }

            case 6:
                break;

            default:
                printf("\nInvalid choice!\n");
        }

        saveCourses();

    } while(choice != 6);

    printf("\nCourse updated successfully!\n");
}


/* =========================================================
   DELETE COURSE
   LINKED LIST DELETION
   ========================================================= */

void deleteCourse()
{
    int id;

    printf("\nEnter Course ID to delete: ");
    scanf("%d", &id);

    Course *temp = courseHead;
    Course *prev = NULL;

    while(temp != NULL && temp->id != id)
    {
        prev = temp;
        temp = temp->next;
    }

    if(temp == NULL)
    {
        printf("\nCourse not found!\n");
        return;
    }

    /*
       LINKED LIST DELETION
    */

    if(prev == NULL)
    {
        courseHead = temp->next;
    }
    else
    {
        prev->next = temp->next;
    }

    free(temp);

    /*
       Applications belonging to deleted course
       are rejected.
    */

    Application *app = applicationHead;

    while(app != NULL)
    {
        if(app->courseID == id)
        {
            app->status = REJECTED;
        }

        app = app->next;
    }

    saveCourses();
    saveApplications();

    printf("\nCourse deleted successfully!\n");
}


/* =========================================================
   FIND COURSE
   LINKED LIST TRAVERSAL
   ========================================================= */

Course* findCourse(int id)
{
    Course *temp = courseHead;

    while(temp != NULL)
    {
        if(temp->id == id)
        {
            return temp;
        }

        temp = temp->next;
    }

    return NULL;
}


/* =========================================================
   APPLY FOR COURSE
   ========================================================= */

void applyCourse(Student *student)
{
    int courseID;

    printf("\nEnter Course ID: ");
    scanf("%d", &courseID);

    Course *course = findCourse(courseID);

    if(course == NULL)
    {
        printf("\nCourse not found!\n");
        return;
    }

    /*
       DUPLICATE APPLICATION CHECK
    */

    Application *temp = applicationHead;

    while(temp != NULL)
    {
        if(temp->studentID == student->id &&
           temp->courseID == courseID)
        {
            if(temp->status != REJECTED)
            {
                printf("\nYou already have an application "
                       "for this course.\n");

                printf("Current Status: %s\n",
                       statusString(temp->status));

                return;
            }
        }

        temp = temp->next;
    }

    /*
       PREREQUISITE CHECK
    */

    if(!checkEligibility(student, course))
    {
        return;
    }

    /*
       CREATE APPLICATION
    */

    Application *newApplication;

    newApplication =
        (Application*)malloc(sizeof(Application));

    if(newApplication == NULL)
    {
        printf("\nMemory allocation failed!\n");
        return;
    }

    newApplication->applicationID =
        nextApplicationID++;

    newApplication->studentID =
        student->id;

    newApplication->courseID =
        courseID;

    /*
       CHECK AVAILABLE SEATS
    */

    int enrolled =
        getEnrolledCount(courseID);

    if(enrolled < course->capacity)
    {
        /*
           Seats available:
           application waits for faculty approval.
        */

        newApplication->status =
            PENDING;

        printf("\nApplication submitted successfully!");
        printf("\nStatus: PENDING");
        printf("\nWaiting for Faculty approval.\n");
    }
    else
    {
        /*
           COURSE FULL:
           DIRECTLY ENTER FIFO WAITLIST
        */

        newApplication->status =
            WAITLISTED;

        printf("\nCourse is FULL.");
        printf("\nYou have been added to the FIFO waitlist.\n");
    }

    newApplication->next = NULL;

    if(applicationHead == NULL)
    {
        applicationHead = newApplication;
    }
    else
    {
        temp = applicationHead;

        while(temp->next != NULL)
        {
            temp = temp->next;
        }

        temp->next = newApplication;
    }

    saveApplications();

    if(newApplication->status == WAITLISTED)
    {
        int position =
            getWaitlistPosition(
                student->id,
                courseID);

        printf("\nYour Waitlist Position: %d\n",
               position);
    }
}


/* =========================================================
   ELIGIBILITY CHECK
   ========================================================= */

int checkEligibility(Student *student,
                     Course *course)
{
    /*
       PREREQUISITE CHECK
    */

    if(course->prerequisiteID != 0)
    {
        if(!hasCompletedPrerequisite(
                student,
                course))
        {
            Course *pre =
                findCourse(course->prerequisiteID);

            printf("\n========== ELIGIBILITY CHECK ==========\n");

            if(pre != NULL)
            {
                printf("You are NOT eligible for this course.\n");

                printf("Required Prerequisite:\n");
                printf("%s (Course ID: %d)\n",
                       pre->name,
                       pre->id);

                printf("\nYou must complete the prerequisite "
                       "course first.\n");
            }

            return 0;
        }
    }

    /*
       MAXIMUM CREDIT CHECK
    */

    int currentCredits =
        getStudentCredits(student);

    if(currentCredits + course->credits >
       MAX_CREDITS)
    {
        printf("\n========== CREDIT LIMIT ==========\n");

        printf("Current Credits : %d\n",
               currentCredits);

        printf("Course Credits  : %d\n",
               course->credits);

        printf("Maximum Credits : %d\n",
               MAX_CREDITS);

        printf("\nYou cannot register for this course.\n");
        printf("Maximum credit limit exceeded.\n");

        return 0;
    }

    printf("\nEligibility check passed.\n");

    return 1;
}


/* =========================================================
   CHECK PREREQUISITE COMPLETION
   ========================================================= */

int hasCompletedPrerequisite(Student *student,
                             Course *course)
{
    Application *app =
        applicationHead;

    while(app != NULL)
    {
        if(app->studentID == student->id &&
           app->courseID == course->prerequisiteID &&
           app->status == COMPLETED)
        {
            return 1;
        }

        app = app->next;
    }

    return 0;
}


/* =========================================================
   GET STUDENT TOTAL CREDITS
   APPROVED + COMPLETED
   ========================================================= */

int getStudentCredits(Student *student)
{
    Application *app =
        applicationHead;

    int totalCredits = 0;

    while(app != NULL)
    {
        if(app->studentID == student->id &&
           (app->status == APPROVED ||
            app->status == COMPLETED))
        {
            Course *course =
                findCourse(app->courseID);

            if(course != NULL)
            {
                totalCredits +=
                    course->credits;
            }
        }

        app = app->next;
    }

    return totalCredits;
}


/* =========================================================
   VIEW STUDENT APPLICATIONS
   ========================================================= */

void viewStudentApplications(Student *student)
{
    Application *app =
        applicationHead;

    int found = 0;

    printf("\n========== MY APPLICATIONS ==========\n");

    while(app != NULL)
    {
        if(app->studentID == student->id)
        {
            Course *course =
                findCourse(app->courseID);

            if(course != NULL)
            {
                printf("\nApplication ID : %d",
                       app->applicationID);

                printf("\nCourse         : %s",
                       course->name);

                printf("\nCredits        : %d",
                       course->credits);

                printf("\nStatus         : %s",
                       statusString(app->status));

                if(app->status == WAITLISTED)
                {
                    int position =
                        getWaitlistPosition(
                            student->id,
                            course->id);

                    printf("\nWaitlist Position : %d",
                           position);
                }

                printf("\n----------------------------------\n");

                found = 1;
            }
        }

        app = app->next;
    }

    if(!found)
    {
        printf("\nNo applications found.\n");
    }
}


/* =========================================================
   VIEW APPROVED / COMPLETED COURSES
   ========================================================= */

void viewApprovedCourses(Student *student)
{
    Application *app =
        applicationHead;

    int found = 0;

    printf("\n========== MY COURSES ==========\n");

    while(app != NULL)
    {
        if(app->studentID == student->id &&
           (app->status == APPROVED ||
            app->status == COMPLETED))
        {
            Course *course =
                findCourse(app->courseID);

            if(course != NULL)
            {
                printf("\nCourse ID   : %d",
                       course->id);

                printf("\nCourse Name : %s",
                       course->name);

                printf("\nCredits     : %d",
                       course->credits);

                printf("\nStatus      : %s",
                       statusString(app->status));

                printf("\n------------------------------\n");

                found = 1;
            }
        }

        app = app->next;
    }

    if(!found)
    {
        printf("\nNo approved or completed courses.\n");
    }
}


/* =========================================================
   ACTUAL STUDENT WAITLIST POSITION
   ========================================================= */

void viewStudentWaitlistPositions(Student *student)
{
    Application *app =
        applicationHead;

    int found = 0;

    printf("\n========== MY WAITLIST POSITIONS ==========\n");

    while(app != NULL)
    {
        if(app->studentID == student->id &&
           app->status == WAITLISTED)
        {
            Course *course =
                findCourse(app->courseID);

            if(course != NULL)
            {
                int position =
                    getWaitlistPosition(
                        student->id,
                        course->id);

                printf("\nCourse ID       : %d",
                       course->id);

                printf("\nCourse Name     : %s",
                       course->name);

                printf("\nStatus          : WAITLISTED");

                printf("\nWaitlist Position : %d",
                       position);

                printf("\n------------------------------\n");

                found = 1;
            }
        }

        app = app->next;
    }

    if(!found)
    {
        printf("\nYou are not currently on any waitlist.\n");
    }
}


/* =========================================================
   GET WAITLIST POSITION
   FIFO ORDER
   ========================================================= */

int getWaitlistPosition(int studentID,
                        int courseID)
{
    Application *app =
        applicationHead;

    int position = 0;

    while(app != NULL)
    {
        if(app->courseID == courseID &&
           app->status == WAITLISTED)
        {
            position++;

            if(app->studentID == studentID)
            {
                return position;
            }
        }

        app = app->next;
    }

    return 0;
}


/* =========================================================
   VIEW FACULTY APPLICATIONS
   ========================================================= */

void viewApplications(Faculty *faculty)
{
    Application *app =
        applicationHead;

    int found = 0;

    printf("\n========== STUDENT APPLICATIONS ==========\n");

    while(app != NULL)
    {
        Course *course =
            findCourse(app->courseID);

        if(course != NULL &&
           course->facultyID == faculty->id)
        {
            Student *student =
                findStudent(app->studentID);

            if(student != NULL)
            {
                printf("\nApplication ID : %d",
                       app->applicationID);

                printf("\nStudent ID     : %d",
                       student->id);

                printf("\nStudent Name   : %s",
                       student->name);

                printf("\nCourse         : %s",
                       course->name);

                printf("\nStatus         : %s",
                       statusString(app->status));

                printf("\n----------------------------------\n");

                found = 1;
            }
        }

        app = app->next;
    }

    if(!found)
    {
        printf("\nNo applications found.\n");
    }
}


/* =========================================================
   APPROVE APPLICATION
   ========================================================= */

void approveApplication(Faculty *faculty)
{
    int applicationID;

    printf("\nEnter Application ID: ");
    scanf("%d", &applicationID);

    Application *app =
        findApplication(applicationID);

    if(app == NULL)
    {
        printf("\nApplication not found!\n");
        return;
    }

    Course *course =
        findCourse(app->courseID);

    if(course == NULL)
    {
        printf("\nCourse not found!\n");
        return;
    }

    if(course->facultyID != faculty->id)
    {
        printf("\nYou are not assigned to this course!\n");
        return;
    }

    if(app->status != PENDING)
    {
        printf("\nOnly PENDING applications can be approved.\n");

        printf("Current status: %s\n",
               statusString(app->status));

        return;
    }

    Student *student =
        findStudent(app->studentID);

    if(student == NULL)
    {
        printf("\nStudent not found!\n");
        return;
    }

    /*
       RECHECK ELIGIBILITY
    */

    if(!checkEligibility(student, course))
    {
        printf("\nApplication cannot be approved "
               "because eligibility requirements are not met.\n");

        app->status = REJECTED;

        saveApplications();

        return;
    }

    /*
       CHECK SEAT
    */

    if(getEnrolledCount(course->id)
       >= course->capacity)
    {
        app->status = WAITLISTED;

        printf("\nCourse became FULL.");
        printf("\nStudent moved to FIFO waitlist.\n");

        printf("Waitlist Position: %d\n",
               getWaitlistPosition(
                   student->id,
                   course->id));

        saveApplications();

        return;
    }

    app->status =
        APPROVED;

    saveApplications();

    printf("\nStudent approved successfully!\n");

    printf("Student: %s\n",
           student->name);

    printf("Course : %s\n",
           course->name);
}


/* =========================================================
   REJECT APPLICATION
   ========================================================= */

void rejectApplication(Faculty *faculty)
{
    int applicationID;

    printf("\nEnter Application ID: ");
    scanf("%d", &applicationID);

    Application *app =
        findApplication(applicationID);

    if(app == NULL)
    {
        printf("\nApplication not found!\n");
        return;
    }

    Course *course =
        findCourse(app->courseID);

    if(course == NULL ||
       course->facultyID != faculty->id)
    {
        printf("\nYou are not assigned to this course!\n");
        return;
    }

    if(app->status != PENDING)
    {
        printf("\nOnly PENDING applications can be rejected.\n");
        return;
    }

    app->status =
        REJECTED;

    saveApplications();

    printf("\nApplication rejected successfully.\n");
}


/* =========================================================
   VIEW ENROLLED STUDENTS
   ========================================================= */

void viewEnrolledStudents(Faculty *faculty)
{
    int courseID;

    printf("\n========== ENROLLED STUDENTS ==========\n");

    printf("\nEnter Course ID: ");
    scanf("%d", &courseID);

    Course *course =
        findCourse(courseID);

    if(course == NULL)
    {
        printf("\nCourse not found!\n");
        return;
    }

    if(course->facultyID != faculty->id)
    {
        printf("\nYou are not assigned to this course!\n");
        return;
    }

    Application *app =
        applicationHead;

    int found = 0;

    while(app != NULL)
    {
        if(app->courseID == courseID &&
           (app->status == APPROVED ||
            app->status == COMPLETED))
        {
            Student *student =
                findStudent(app->studentID);

            if(student != NULL)
            {
                printf("\nStudent ID   : %d",
                       student->id);

                printf("\nStudent Name : %s",
                       student->name);

                printf("\nEmail        : %s",
                       student->email);

                printf("\nStatus       : %s",
                       statusString(app->status));

                printf("\n---------------------------\n");

                found = 1;
            }
        }

        app = app->next;
    }

    if(!found)
    {
        printf("\nNo enrolled students.\n");
        return;
    }

    /*
       REMOVE OPTION IS INSIDE
       ENROLLED STUDENTS.
    */

    int choice;

    printf("\n1. Remove Student");
    printf("\n2. Back");

    printf("\nEnter choice: ");
    scanf("%d", &choice);

    if(choice == 1)
    {
        removeStudentFromCourse(faculty);
    }
}


/* =========================================================
   REMOVE STUDENT FROM COURSE
   ========================================================= */

void removeStudentFromCourse(Faculty *faculty)
{
    int studentID;
    int courseID;

    printf("\nEnter Student ID to remove: ");
    scanf("%d", &studentID);

    printf("Enter Course ID: ");
    scanf("%d", &courseID);

    Course *course =
        findCourse(courseID);

    if(course == NULL ||
       course->facultyID != faculty->id)
    {
        printf("\nInvalid course or faculty assignment!\n");
        return;
    }

    Application *app =
        applicationHead;

    while(app != NULL)
    {
        if(app->studentID == studentID &&
           app->courseID == courseID &&
           (app->status == APPROVED ||
            app->status == COMPLETED))
        {
            /*
               STUDENT IS REMOVED.

               Students do not have a DROP option.
               Faculty controls removal.
            */

            app->status =
                REJECTED;

            printf("\nStudent removed from course successfully!\n");

            /*
               AUTOMATIC FIFO PROMOTION
            */

            promoteWaitlistedStudent(course);

            saveApplications();

            return;
        }

        app = app->next;
    }

    printf("\nStudent is not enrolled in this course.\n");
}


/* =========================================================
   MARK COURSE COMPLETED
   USED FOR PREREQUISITE CHECKING
   ========================================================= */

void markCourseCompleted(Faculty *faculty)
{
    int studentID;
    int courseID;

    printf("\n========== MARK COURSE COMPLETED ==========\n");

    printf("Enter Student ID: ");
    scanf("%d", &studentID);

    printf("Enter Course ID: ");
    scanf("%d", &courseID);

    Course *course =
        findCourse(courseID);

    if(course == NULL)
    {
        printf("\nCourse not found!\n");
        return;
    }

    if(course->facultyID != faculty->id)
    {
        printf("\nYou are not assigned to this course!\n");
        return;
    }

    Application *app =
        applicationHead;

    while(app != NULL)
    {
        if(app->studentID == studentID &&
           app->courseID == courseID &&
           app->status == APPROVED)
        {
            app->status =
                COMPLETED;

            saveApplications();

            printf("\nCourse marked as COMPLETED.\n");

            printf("Student can now use this course "
                   "as a prerequisite.\n");

            return;
        }

        app = app->next;
    }

    printf("\nStudent is not currently APPROVED "
           "for this course.\n");
}


/* =========================================================
   PROMOTE WAITLISTED STUDENT
   ========================================================= */

void promoteWaitlistedStudent(Course *course)
{
    if(getEnrolledCount(course->id)
       >= course->capacity)
    {
        return;
    }

    Queue q;

    initializeQueue(&q);

    buildWaitlist(course->id, &q);

    /*
       FIFO:
       Remove first student from front.
    */

    WaitNode *node =
        dequeue(&q);

    if(node == NULL)
    {
        printf("\nNo students in waitlist.\n");
        return;
    }

    Application *app =
        findApplication(node->applicationID);

    if(app != NULL)
    {
        Student *student =
            findStudent(app->studentID);

        /*
           Recheck eligibility before promotion.
        */

        if(student != NULL &&
           checkEligibility(student, course))
        {
            app->status =
                APPROVED;

            printf("\n========== FIFO PROMOTION ==========\n");

            printf("Student %s has been promoted.\n",
                   student->name);

            printf("Student ID : %d\n",
                   student->id);

            printf("Course     : %s\n",
                   course->name);
        }
        else
        {
            /*
               If first student is no longer eligible,
               reject and check the next student.
            */

            app->status =
                REJECTED;

            printf("\nFirst waitlisted student is "
                   "no longer eligible.\n");

            free(node);

            /*
               Try next FIFO student.
            */

            promoteWaitlistedStudent(course);

            return;
        }
    }

    free(node);

    saveApplications();
}


/* =========================================================
   VIEW COURSE WAITLIST
   ========================================================= */

void viewCourseWaitlist(Faculty *faculty)
{
    int courseID;

    printf("\nEnter Course ID: ");
    scanf("%d", &courseID);

    Course *course =
        findCourse(courseID);

    if(course == NULL)
    {
        printf("\nCourse not found!\n");
        return;
    }

    if(course->facultyID != faculty->id)
    {
        printf("\nYou are not assigned to this course!\n");
        return;
    }

    Queue q;

    initializeQueue(&q);

    buildWaitlist(courseID, &q);

    if(q.front == NULL)
    {
        printf("\nWaitlist is empty.\n");
        return;
    }

    printf("\n========== FIFO WAITLIST ==========\n");

    int position = 1;

    WaitNode *temp =
        q.front;

    while(temp != NULL)
    {
        Student *student =
            findStudent(temp->studentID);

        if(student != NULL)
        {
            printf("\nPosition     : %d",
                   position);

            printf("\nStudent ID   : %d",
                   student->id);

            printf("\nStudent Name : %s",
                   student->name);

            printf("\n---------------------------\n");
        }

        position++;

        temp = temp->next;
    }
}


/* =========================================================
   GET ENROLLED COUNT
   ========================================================= */

int getEnrolledCount(int courseID)
{
    Application *app =
        applicationHead;

    int count = 0;

    while(app != NULL)
    {
        if(app->courseID == courseID &&
           (app->status == APPROVED ||
            app->status == COMPLETED))
        {
            count++;
        }

        app = app->next;
    }

    return count;
}


/* =========================================================
   QUEUE INITIALIZATION
   ========================================================= */

void initializeQueue(Queue *q)
{
    q->front = NULL;
    q->rear = NULL;
}


/* =========================================================
   ENQUEUE
   FIFO
   ========================================================= */

void enqueue(Queue *q,
             int applicationID,
             int studentID,
             int courseID)
{
    WaitNode *newNode;

    newNode =
        (WaitNode*)malloc(sizeof(WaitNode));

    if(newNode == NULL)
    {
        return;
    }

    newNode->applicationID =
        applicationID;

    newNode->studentID =
        studentID;

    newNode->courseID =
        courseID;

    newNode->next = NULL;

    if(q->rear == NULL)
    {
        q->front =
            q->rear =
            newNode;
    }
    else
    {
        q->rear->next =
            newNode;

        q->rear =
            newNode;
    }
}


/* =========================================================
   DEQUEUE
   FIFO
   ========================================================= */

WaitNode* dequeue(Queue *q)
{
    if(q->front == NULL)
    {
        return NULL;
    }

    WaitNode *temp =
        q->front;

    q->front =
        q->front->next;

    if(q->front == NULL)
    {
        q->rear = NULL;
    }

    temp->next = NULL;

    return temp;
}


/* =========================================================
   BUILD WAITLIST QUEUE
   ========================================================= */

void buildWaitlist(int courseID,
                   Queue *q)
{
    Application *app =
        applicationHead;

    while(app != NULL)
    {
        if(app->courseID == courseID &&
           app->status == WAITLISTED)
        {
            enqueue(q,
                    app->applicationID,
                    app->studentID,
                    app->courseID);
        }

        app = app->next;
    }
}


/* =========================================================
   FIND STUDENT
   ========================================================= */

Student* findStudent(int id)
{
    Student *temp =
        studentHead;

    while(temp != NULL)
    {
        if(temp->id == id)
        {
            return temp;
        }

        temp = temp->next;
    }

    return NULL;
}


/* =========================================================
   FIND FACULTY
   ========================================================= */

Faculty* findFaculty(int id)
{
    Faculty *temp =
        facultyHead;

    while(temp != NULL)
    {
        if(temp->id == id)
        {
            return temp;
        }

        temp = temp->next;
    }

    return NULL;
}


/* =========================================================
   FIND APPLICATION
   ========================================================= */

Application* findApplication(int id)
{
    Application *temp =
        applicationHead;

    while(temp != NULL)
    {
        if(temp->applicationID == id)
        {
            return temp;
        }

        temp = temp->next;
    }

    return NULL;
}


/* =========================================================
   STATUS STRING
   ========================================================= */

const char* statusString(Status status)
{
    switch(status)
    {
        case PENDING:
            return "PENDING";

        case APPROVED:
            return "APPROVED";

        case REJECTED:
            return "REJECTED";

        case WAITLISTED:
            return "WAITLISTED";

        case COMPLETED:
            return "COMPLETED";

        default:
            return "UNKNOWN";
    }
}


/* =========================================================
   DISPLAY ALL STUDENTS
   ========================================================= */

void displayAllStudents()
{
    Student *temp =
        studentHead;

    printf("\n========== ALL STUDENTS ==========\n");

    if(temp == NULL)
    {
        printf("\nNo students found.\n");
        return;
    }

    while(temp != NULL)
    {
        printf("\nStudent ID : %d",
               temp->id);

        printf("\nName       : %s",
               temp->name);

        printf("\nEmail      : %s",
               temp->email);

        printf("\nCredits    : %d / %d",
               getStudentCredits(temp),
               MAX_CREDITS);

        printf("\n----------------------------\n");

        temp = temp->next;
    }
}


/* =========================================================
   DISPLAY ALL FACULTY
   ========================================================= */

void displayAllFaculty()
{
    Faculty *temp =
        facultyHead;

    printf("\n========== ALL FACULTY ==========\n");

    if(temp == NULL)
    {
        printf("\nNo faculty found.\n");
        return;
    }

    while(temp != NULL)
    {
        printf("\nFaculty ID : %d",
               temp->id);

        printf("\nName       : %s",
               temp->name);

        printf("\nEmail      : %s",
               temp->email);

        printf("\n----------------------------\n");

        temp = temp->next;
    }
}


/* =========================================================
   DISPLAY ALL APPLICATIONS
   ========================================================= */

void displayAllApplications()
{
    Application *app =
        applicationHead;

    printf("\n========== ALL APPLICATIONS ==========\n");

    if(app == NULL)
    {
        printf("\nNo applications found.\n");
        return;
    }

    while(app != NULL)
    {
        Student *student =
            findStudent(app->studentID);

        Course *course =
            findCourse(app->courseID);

        printf("\nApplication ID : %d",
               app->applicationID);

        if(student != NULL)
        {
            printf("\nStudent        : %s",
                   student->name);
        }

        if(course != NULL)
        {
            printf("\nCourse         : %s",
                   course->name);
        }

        printf("\nStatus         : %s",
               statusString(app->status));

        if(app->status == WAITLISTED)
        {
            printf("\nWaitlist Pos.  : %d",
                   getWaitlistPosition(
                       app->studentID,
                       app->courseID));
        }

        printf("\n----------------------------\n");

        app = app->next;
    }
}


/* =========================================================
   DISPLAY ALL WAITLISTS
   ========================================================= */

void displayAllWaitlists()
{
    Course *course =
        courseHead;

    printf("\n========== ALL COURSE WAITLISTS ==========\n");

    if(course == NULL)
    {
        printf("\nNo courses found.\n");
        return;
    }

    while(course != NULL)
    {
        Queue q;

        initializeQueue(&q);

        buildWaitlist(course->id,
                      &q);

        printf("\nCourse: %s",
               course->name);

        if(q.front == NULL)
        {
            printf("\nWaitlist: Empty\n");
        }
        else
        {
            WaitNode *temp =
                q.front;

            int position = 1;

            while(temp != NULL)
            {
                Student *student =
                    findStudent(
                        temp->studentID);

                if(student != NULL)
                {
                    printf("\n%d. %s (ID: %d)",
                           position,
                           student->name,
                           student->id);
                }

                position++;

                temp = temp->next;
            }

            printf("\n");
        }

        printf("----------------------------------\n");

        course = course->next;
    }
}


/* =========================================================
   SAVE STUDENTS
   ========================================================= */

void saveStudents()
{
    FILE *fp =
        fopen("students.txt", "w");

    if(fp == NULL)
    {
        return;
    }

    Student *temp =
        studentHead;

    while(temp != NULL)
    {
        fprintf(fp,
                "%d|%s|%s|%s\n",
                temp->id,
                temp->name,
                temp->email,
                temp->password);

        temp = temp->next;
    }

    fclose(fp);
}


/* =========================================================
   SAVE FACULTY
   ========================================================= */

void saveFaculty()
{
    FILE *fp =
        fopen("faculty.txt", "w");

    if(fp == NULL)
    {
        return;
    }

    Faculty *temp =
        facultyHead;

    while(temp != NULL)
    {
        fprintf(fp,
                "%d|%s|%s|%s\n",
                temp->id,
                temp->name,
                temp->email,
                temp->password);

        temp = temp->next;
    }

    fclose(fp);
}


/* =========================================================
   SAVE COURSES
   ========================================================= */

void saveCourses()
{
    FILE *fp =
        fopen("courses.txt", "w");

    if(fp == NULL)
    {
        return;
    }

    Course *temp =
        courseHead;

    while(temp != NULL)
    {
        /*
           Format:

           ID|NAME|CREDITS|CAPACITY|FACULTYID|PREREQUISITEID
        */

        fprintf(fp,
                "%d|%s|%d|%d|%d|%d\n",
                temp->id,
                temp->name,
                temp->credits,
                temp->capacity,
                temp->facultyID,
                temp->prerequisiteID);

        temp = temp->next;
    }

    fclose(fp);
}


/* =========================================================
   SAVE APPLICATIONS
   ========================================================= */

void saveApplications()
{
    FILE *fp =
        fopen("applications.txt", "w");

    if(fp == NULL)
    {
        return;
    }

    Application *temp =
        applicationHead;

    while(temp != NULL)
    {
        /*
           Format:

           ApplicationID|StudentID|CourseID|Status
        */

        fprintf(fp,
                "%d|%d|%d|%d\n",
                temp->applicationID,
                temp->studentID,
                temp->courseID,
                temp->status);

        temp = temp->next;
    }

    fclose(fp);
}


/* =========================================================
   SAVE ALL
   ========================================================= */

void saveAll()
{
    saveStudents();
    saveFaculty();
    saveCourses();
    saveApplications();
}


/* =========================================================
   LOAD STUDENTS
   ========================================================= */

void loadStudents()
{
    FILE *fp =
        fopen("students.txt", "r");

    if(fp == NULL)
    {
        return;
    }

    Student *newStudent;

    while(1)
    {
        newStudent =
            (Student*)malloc(sizeof(Student));

        if(newStudent == NULL)
        {
            break;
        }

        if(fscanf(fp,
                  "%d|%49[^|]|%59[^|]|%29[^\n]\n",
                  &newStudent->id,
                  newStudent->name,
                  newStudent->email,
                  newStudent->password) != 4)
        {
            free(newStudent);
            break;
        }

        newStudent->next = NULL;

        if(studentHead == NULL)
        {
            studentHead = newStudent;
        }
        else
        {
            Student *temp =
                studentHead;

            while(temp->next != NULL)
            {
                temp = temp->next;
            }

            temp->next =
                newStudent;
        }
    }

    fclose(fp);
}


/* =========================================================
   LOAD FACULTY
   ========================================================= */

void loadFaculty()
{
    FILE *fp =
        fopen("faculty.txt", "r");

    if(fp == NULL)
    {
        return;
    }

    Faculty *newFaculty;

    while(1)
    {
        newFaculty =
            (Faculty*)malloc(sizeof(Faculty));

        if(newFaculty == NULL)
        {
            break;
        }

        if(fscanf(fp,
                  "%d|%49[^|]|%59[^|]|%29[^\n]\n",
                  &newFaculty->id,
                  newFaculty->name,
                  newFaculty->email,
                  newFaculty->password) != 4)
        {
            free(newFaculty);
            break;
        }

        newFaculty->next = NULL;

        if(facultyHead == NULL)
        {
            facultyHead =
                newFaculty;
        }
        else
        {
            Faculty *temp =
                facultyHead;

            while(temp->next != NULL)
            {
                temp = temp->next;
            }

            temp->next =
                newFaculty;
        }
    }

    fclose(fp);
}


/* =========================================================
   LOAD COURSES
   ========================================================= */

void loadCourses()
{
    FILE *fp =
        fopen("courses.txt", "r");

    if(fp == NULL)
    {
        return;
    }

    Course *newCourse;

    while(1)
    {
        newCourse =
            (Course*)malloc(sizeof(Course));

        if(newCourse == NULL)
        {
            break;
        }

        /*
           New format:

           ID|NAME|CREDITS|CAPACITY|FACULTYID|PREREQUISITEID
        */

        if(fscanf(fp,
                  "%d|%79[^|]|%d|%d|%d|%d\n",
                  &newCourse->id,
                  newCourse->name,
                  &newCourse->credits,
                  &newCourse->capacity,
                  &newCourse->facultyID,
                  &newCourse->prerequisiteID) != 6)
        {
            free(newCourse);
            break;
        }

        newCourse->next = NULL;

        if(courseHead == NULL)
        {
            courseHead =
                newCourse;
        }
        else
        {
            Course *temp =
                courseHead;

            while(temp->next != NULL)
            {
                temp = temp->next;
            }

            temp->next =
                newCourse;
        }
    }

    fclose(fp);
}


/* =========================================================
   LOAD APPLICATIONS
   ========================================================= */

void loadApplications()
{
    FILE *fp =
        fopen("applications.txt", "r");

    if(fp == NULL)
    {
        return;
    }

    Application *newApplication;

    while(1)
    {
        newApplication =
            (Application*)malloc(sizeof(Application));

        if(newApplication == NULL)
        {
            break;
        }

        int status;

        if(fscanf(fp,
                  "%d|%d|%d|%d\n",
                  &newApplication->applicationID,
                  &newApplication->studentID,
                  &newApplication->courseID,
                  &status) != 4)
        {
            free(newApplication);
            break;
        }

        newApplication->status =
            (Status)status;

        newApplication->next = NULL;

        if(applicationHead == NULL)
        {
            applicationHead =
                newApplication;
        }
        else
        {
            Application *temp =
                applicationHead;

            while(temp->next != NULL)
            {
                temp = temp->next;
            }

            temp->next =
                newApplication;
        }

        if(newApplication->applicationID
           >= nextApplicationID)
        {
            nextApplicationID =
                newApplication->applicationID + 1;
        }
    }

    fclose(fp);
}


/* =========================================================
   LOAD ALL
   ========================================================= */

void loadAll()
{
    loadStudents();
    loadFaculty();
    loadCourses();
    loadApplications();
}
