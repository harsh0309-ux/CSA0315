from flask import Flask, render_template, request, redirect, session, flash
from db import cursor, db

app = Flask(__name__)

app.secret_key = "course_registration_project"


# =========================================================
# HELPER FUNCTIONS
# =========================================================

def student_required():
    return "student_id" in session


def instructor_required():
    return "instructor_id" in session


def admin_required():
    return "admin" in session


# =========================================================
# PREREQUISITE CHECK
# =========================================================

def prerequisite_completed(student_id, course_id):

    cursor.execute("""
        SELECT prerequisite_course_id
        FROM prerequisites
        WHERE course_id=%s
    """, (course_id,))

    prereqs = cursor.fetchall()

    if not prereqs:
        return True

    for row in prereqs:

        prerequisite = row[0]

        cursor.execute("""
            SELECT *
            FROM enrollments
            WHERE student_id=%s
            AND course_id=%s
            AND status='Approved'
        """, (student_id, prerequisite))

        if cursor.fetchone() is None:
            return False

    return True


# =========================================================
# HOME
# =========================================================

@app.route("/")
def home():
    return render_template("index.html")


# =========================================================
# STUDENT REGISTER
# =========================================================

@app.route("/register", methods=["GET", "POST"])
def register():

    if request.method == "POST":

        full_name = request.form["full_name"]
        registration_no = request.form["registration_no"]
        email = request.form["email"]
        password = request.form["password"]

        try:

            cursor.execute("""
                INSERT INTO students
                (
                    full_name,
                    registration_no,
                    email,
                    password
                )
                VALUES
                (%s, %s, %s, %s)
            """, (
                full_name,
                registration_no,
                email,
                password
            ))

            db.commit()

            flash(
                "Student account created successfully.",
                "success"
            )

            return redirect("/login")

        except Exception:

            db.rollback()

            flash(
                "Registration failed. Email or registration number may already exist.",
                "danger"
            )

    return render_template("register.html")


# =========================================================
# STUDENT LOGIN
# =========================================================

@app.route("/login", methods=["GET", "POST"])
def login():

    if request.method == "POST":

        email = request.form["email"]
        password = request.form["password"]

        cursor.execute("""
            SELECT *
            FROM students
            WHERE email=%s
            AND password=%s
        """, (
            email,
            password
        ))

        student = cursor.fetchone()

        if student:

            session.clear()

            session["student_id"] = student[0]
            session["student_name"] = student[1]

            flash(
                "Login successful.",
                "success"
            )

            return redirect("/student_dashboard")

        flash(
            "Invalid email or password.",
            "danger"
        )

    return render_template("login.html")


# =========================================================
# INSTRUCTOR LOGIN
# =========================================================

@app.route("/instructor_login", methods=["GET", "POST"])
def instructor_login():

    if request.method == "POST":

        email = request.form["email"]
        password = request.form["password"]

        cursor.execute("""
            SELECT *
            FROM instructors
            WHERE email=%s
            AND password=%s
        """, (
            email,
            password
        ))

        instructor = cursor.fetchone()

        if instructor:

            session.clear()

            session["instructor_id"] = instructor[0]
            session["instructor_name"] = instructor[1]

            flash(
                "Welcome to the instructor portal.",
                "success"
            )

            return redirect("/instructor_dashboard")

        flash(
            "Invalid email or password.",
            "danger"
        )

    return render_template("instructor_login.html")


# =========================================================
# ADMIN LOGIN
# =========================================================

@app.route("/admin_login", methods=["GET", "POST"])
def admin_login():

    if request.method == "POST":

        username = request.form["username"]
        password = request.form["password"]

        cursor.execute("""
            SELECT *
            FROM admin
            WHERE username=%s
            AND password=%s
        """, (
            username,
            password
        ))

        admin_user = cursor.fetchone()

        if admin_user:

            session.clear()

            session["admin"] = admin_user[1]

            flash(
                "Welcome to the administrator portal.",
                "success"
            )

            return redirect("/admin_dashboard")

        flash(
            "Invalid username or password.",
            "danger"
        )

    return render_template("admin_login.html")


# =========================================================
# STUDENT DASHBOARD
# =========================================================

@app.route("/student_dashboard")
def student_dashboard():

    if not student_required():
        return redirect("/login")

    student_id = session["student_id"]

    cursor.execute("""
        SELECT *
        FROM students
        WHERE student_id=%s
    """, (student_id,))

    student = cursor.fetchone()

    if student is None:

        session.clear()

        flash(
            "Student account not found.",
            "danger"
        )

        return redirect("/login")

    cursor.execute("""
        SELECT COUNT(*)
        FROM enrollments
        WHERE student_id=%s
    """, (student_id,))

    total = cursor.fetchone()[0]

    cursor.execute("""
        SELECT COUNT(*)
        FROM enrollments
        WHERE student_id=%s
        AND status='Approved'
    """, (student_id,))

    approved = cursor.fetchone()[0]

    cursor.execute("""
        SELECT COUNT(*)
        FROM enrollments
        WHERE student_id=%s
        AND status='Pending'
    """, (student_id,))

    pending = cursor.fetchone()[0]

    cursor.execute("""
        SELECT COUNT(*)
        FROM enrollments
        WHERE student_id=%s
        AND status='Rejected'
    """, (student_id,))

    rejected = cursor.fetchone()[0]

    cursor.execute("""
        SELECT COUNT(*)
        FROM waitlist
        WHERE student_id=%s
    """, (student_id,))

    waitlisted = cursor.fetchone()[0]

    return render_template(
        "student_dashboard.html",
        student=student,
        student_name=session.get("student_name"),
        total=total,
        approved=approved,
        pending=pending,
        rejected=rejected,
        waitlisted=waitlisted
    )


# =========================================================
# AVAILABLE COURSES
# =========================================================

@app.route("/available_courses")
def available_courses():

    if not student_required():
        return redirect("/login")

    search = request.args.get("search", "").strip()

    if search:

        cursor.execute("""
            SELECT
                c.course_id,
                c.course_code,
                c.course_name,
                c.credits,
                c.department,
                c.capacity,
                c.available_seats,
                c.instructor_id,
                i.instructor_name
            FROM courses c
            LEFT JOIN instructors i
            ON c.instructor_id=i.instructor_id
            WHERE
                c.course_code LIKE %s
                OR c.course_name LIKE %s
                OR c.department LIKE %s
            ORDER BY c.course_id
        """, (
            f"%{search}%",
            f"%{search}%",
            f"%{search}%"
        ))

    else:

        cursor.execute("""
            SELECT
                c.course_id,
                c.course_code,
                c.course_name,
                c.credits,
                c.department,
                c.capacity,
                c.available_seats,
                c.instructor_id,
                i.instructor_name
            FROM courses c
            LEFT JOIN instructors i
            ON c.instructor_id=i.instructor_id
            ORDER BY c.course_id
        """)

    courses = cursor.fetchall()

    return render_template(
        "available_courses.html",
        courses=courses,
        search=search
    )


# =========================================================
# REGISTER COURSE
# =========================================================

@app.route("/register_course/<int:course_id>")
def register_course(course_id):

    if not student_required():
        return redirect("/login")

    student_id = session["student_id"]

    if not prerequisite_completed(
        student_id,
        course_id
    ):

        flash(
            "You must complete the prerequisite course first.",
            "danger"
        )

        return redirect("/available_courses")

    cursor.execute("""
        SELECT *
        FROM enrollments
        WHERE student_id=%s
        AND course_id=%s
    """, (
        student_id,
        course_id
    ))

    if cursor.fetchone():

        flash(
            "You have already registered for this course.",
            "warning"
        )

        return redirect("/available_courses")

    cursor.execute("""
        SELECT available_seats
        FROM courses
        WHERE course_id=%s
    """, (course_id,))

    course = cursor.fetchone()

    if course is None:

        flash(
            "Course not found.",
            "danger"
        )

        return redirect("/available_courses")

    available = course[0]

    if available > 0:

        cursor.execute("""
            INSERT INTO enrollments
            (
                student_id,
                course_id,
                enrollment_date,
                status
            )
            VALUES
            (
                %s,
                %s,
                CURDATE(),
                'Pending'
            )
        """, (
            student_id,
            course_id
        ))

        db.commit()

        flash(
            "Course registration request submitted.",
            "success"
        )

    else:

        cursor.execute("""
            SELECT COUNT(*)
            FROM waitlist
            WHERE course_id=%s
        """, (course_id,))

        position = cursor.fetchone()[0] + 1

        cursor.execute("""
            INSERT INTO waitlist
            (
                student_id,
                course_id,
                position_no
            )
            VALUES
            (
                %s,
                %s,
                %s
            )
        """, (
            student_id,
            course_id,
            position
        ))

        db.commit()

        flash(
            f"Course is full. You were added to waitlist position {position}.",
            "warning"
        )

    return redirect("/available_courses")


# =========================================================
# MY COURSES
# =========================================================

@app.route("/my_courses")
def my_courses():

    if not student_required():
        return redirect("/login")

    cursor.execute("""
        SELECT
            e.enrollment_id,
            c.course_code,
            c.course_name,
            c.credits,
            i.instructor_name,
            e.enrollment_date,
            e.status
        FROM enrollments e
        JOIN courses c
        ON e.course_id=c.course_id
        LEFT JOIN instructors i
        ON c.instructor_id=i.instructor_id
        WHERE e.student_id=%s
        ORDER BY e.enrollment_date DESC
    """, (
        session["student_id"],
    ))

    courses = cursor.fetchall()

    return render_template(
        "my_courses.html",
        courses=courses
    )


# =========================================================
# DROP COURSE
# =========================================================

@app.route("/drop_course/<int:enrollment_id>")
def drop_course(enrollment_id):

    if not student_required():
        return redirect("/login")

    student_id = session["student_id"]

    cursor.execute("""
        SELECT
            course_id,
            status
        FROM enrollments
        WHERE enrollment_id=%s
        AND student_id=%s
    """, (
        enrollment_id,
        student_id
    ))

    row = cursor.fetchone()

    if row is None:

        flash(
            "Enrollment not found.",
            "danger"
        )

        return redirect("/my_courses")

    course_id = row[0]
    status = row[1]

    cursor.execute("""
        DELETE FROM enrollments
        WHERE enrollment_id=%s
        AND student_id=%s
    """, (
        enrollment_id,
        student_id
    ))

    if status == "Approved":

        cursor.execute("""
            UPDATE courses
            SET available_seats=available_seats+1
            WHERE course_id=%s
        """, (course_id,))

        cursor.execute("""
            SELECT
                student_id,
                position_no
            FROM waitlist
            WHERE course_id=%s
            ORDER BY position_no
            LIMIT 1
        """, (course_id,))

        first = cursor.fetchone()

        if first:

            wait_student = first[0]
            position = first[1]

            cursor.execute("""
                DELETE FROM waitlist
                WHERE course_id=%s
                AND position_no=%s
            """, (
                course_id,
                position
            ))

            cursor.execute("""
                INSERT INTO enrollments
                (
                    student_id,
                    course_id,
                    enrollment_date,
                    status
                )
                VALUES
                (
                    %s,
                    %s,
                    CURDATE(),
                    'Approved'
                )
            """, (
                wait_student,
                course_id
            ))

            cursor.execute("""
                UPDATE courses
                SET available_seats=available_seats-1
                WHERE course_id=%s
            """, (course_id,))

            cursor.execute("""
                SELECT waitlist_id
                FROM waitlist
                WHERE course_id=%s
                ORDER BY position_no
            """, (course_id,))

            remaining = cursor.fetchall()

            position_no = 1

            for item in remaining:

                cursor.execute("""
                    UPDATE waitlist
                    SET position_no=%s
                    WHERE waitlist_id=%s
                """, (
                    position_no,
                    item[0]
                ))

                position_no += 1

    db.commit()

    flash(
        "Course dropped successfully.",
        "success"
    )

    return redirect("/my_courses")


# =========================================================
# STUDENT PROFILE
# =========================================================

@app.route("/profile")
def profile():

    if not student_required():
        return redirect("/login")

    cursor.execute("""
        SELECT *
        FROM students
        WHERE student_id=%s
    """, (
        session["student_id"],
    ))

    student = cursor.fetchone()

    return render_template(
        "profile.html",
        student=student
    )


# =========================================================
# INSTRUCTOR DASHBOARD
# =========================================================

@app.route("/instructor_dashboard")
def instructor_dashboard():

    if not instructor_required():
        return redirect("/instructor_login")

    instructor_id = session["instructor_id"]

    cursor.execute("""
        SELECT COUNT(*)
        FROM courses
        WHERE instructor_id=%s
    """, (instructor_id,))

    total_courses = cursor.fetchone()[0]

    cursor.execute("""
        SELECT COUNT(*)
        FROM enrollments e
        JOIN courses c
        ON e.course_id=c.course_id
        WHERE c.instructor_id=%s
        AND e.status='Pending'
    """, (instructor_id,))

    pending = cursor.fetchone()[0]

    cursor.execute("""
        SELECT COUNT(*)
        FROM enrollments e
        JOIN courses c
        ON e.course_id=c.course_id
        WHERE c.instructor_id=%s
        AND e.status='Approved'
    """, (instructor_id,))

    approved = cursor.fetchone()[0]

    cursor.execute("""
        SELECT COUNT(*)
        FROM enrollments e
        JOIN courses c
        ON e.course_id=c.course_id
        WHERE c.instructor_id=%s
        AND e.status='Rejected'
    """, (instructor_id,))

    rejected = cursor.fetchone()[0]

    return render_template(
        "instructor_dashboard.html",
        instructor_name=session["instructor_name"],
        total_courses=total_courses,
        pending=pending,
        approved=approved,
        rejected=rejected
    )


# =========================================================
# ASSIGNED COURSES
# =========================================================

@app.route("/assigned_courses")
def assigned_courses():

    if not instructor_required():
        return redirect("/instructor_login")

    cursor.execute("""
        SELECT
            course_id,
            course_code,
            course_name,
            credits,
            department,
            capacity,
            available_seats
        FROM courses
        WHERE instructor_id=%s
        ORDER BY course_id
    """, (
        session["instructor_id"],
    ))

    courses = cursor.fetchall()

    return render_template(
        "assigned_courses.html",
        courses=courses
    )


# Alias
@app.route("/instructor_courses")
def instructor_courses():
    return assigned_courses()


# =========================================================
# INSTRUCTOR REQUESTS
# =========================================================

@app.route("/course_requests")
def course_requests():

    if not instructor_required():
        return redirect("/instructor_login")

    cursor.execute("""
        SELECT
            e.enrollment_id,
            s.full_name,
            s.registration_no,
            c.course_name,
            e.enrollment_date,
            e.status
        FROM enrollments e
        JOIN students s
        ON e.student_id=s.student_id
        JOIN courses c
        ON e.course_id=c.course_id
        WHERE c.instructor_id=%s
        ORDER BY e.enrollment_date DESC
    """, (
        session["instructor_id"],
    ))

    requests = cursor.fetchall()

    return render_template(
        "course_requests.html",
        requests=requests
    )


@app.route("/instructor_requests")
def instructor_requests():
    return course_requests()


# =========================================================
# APPROVE REQUEST
# =========================================================

@app.route("/approve_request/<int:enrollment_id>")
def approve_request(enrollment_id):

    if not instructor_required():
        return redirect("/instructor_login")

    cursor.execute("""
        SELECT
            e.course_id
        FROM enrollments e
        JOIN courses c
        ON e.course_id=c.course_id
        WHERE e.enrollment_id=%s
        AND c.instructor_id=%s
        AND e.status='Pending'
    """, (
        enrollment_id,
        session["instructor_id"]
    ))

    row = cursor.fetchone()

    if row is None:

        flash(
            "Request not found.",
            "danger"
        )

        return redirect("/course_requests")

    course_id = row[0]

    cursor.execute("""
        SELECT available_seats
        FROM courses
        WHERE course_id=%s
    """, (course_id,))

    available = cursor.fetchone()[0]

    if available <= 0:

        flash(
            "Course is currently full.",
            "warning"
        )

        return redirect("/course_requests")

    cursor.execute("""
        UPDATE enrollments
        SET status='Approved'
        WHERE enrollment_id=%s
    """, (enrollment_id,))

    cursor.execute("""
        UPDATE courses
        SET available_seats=available_seats-1
        WHERE course_id=%s
    """, (course_id,))

    db.commit()

    flash(
        "Student request approved successfully.",
        "success"
    )

    return redirect("/course_requests")


# =========================================================
# REJECT REQUEST
# =========================================================

@app.route("/reject_request/<int:enrollment_id>")
def reject_request(enrollment_id):

    if not instructor_required():
        return redirect("/instructor_login")

    cursor.execute("""
        UPDATE enrollments e
        JOIN courses c
        ON e.course_id=c.course_id
        SET e.status='Rejected'
        WHERE e.enrollment_id=%s
        AND c.instructor_id=%s
        AND e.status='Pending'
    """, (
        enrollment_id,
        session["instructor_id"]
    ))

    db.commit()

    flash(
        "Student request rejected.",
        "warning"
    )

    return redirect("/course_requests")


# =========================================================
# ADMIN DASHBOARD
# =========================================================

@app.route("/admin_dashboard")
def admin_dashboard():

    if not admin_required():
        return redirect("/admin_login")

    cursor.execute(
        "SELECT COUNT(*) FROM courses"
    )

    total_courses = cursor.fetchone()[0]

    cursor.execute(
        "SELECT COUNT(*) FROM students"
    )

    total_students = cursor.fetchone()[0]

    cursor.execute(
        "SELECT COUNT(*) FROM instructors"
    )

    total_instructors = cursor.fetchone()[0]

    cursor.execute(
        "SELECT COUNT(*) FROM enrollments WHERE status='Pending'"
    )

    total_requests = cursor.fetchone()[0]

    return render_template(
        "admin_dashboard.html",
        total_courses=total_courses,
        total_students=total_students,
        total_instructors=total_instructors,
        total_requests=total_requests
    )


# =========================================================
# ADD COURSE
# =========================================================

@app.route("/add_course", methods=["GET", "POST"])
def add_course():

    if not admin_required():
        return redirect("/admin_login")

    if request.method == "POST":

        course_code = request.form["course_code"]
        course_name = request.form["course_name"]
        credits = request.form["credits"]
        department = request.form["department"]
        capacity = request.form["capacity"]
        instructor_id = request.form["instructor_id"]

        try:

            cursor.execute("""
                INSERT INTO courses
                (
                    course_code,
                    course_name,
                    credits,
                    department,
                    capacity,
                    available_seats,
                    instructor_id
                )
                VALUES
                (%s,%s,%s,%s,%s,%s,%s)
            """, (
                course_code,
                course_name,
                credits,
                department,
                capacity,
                capacity,
                instructor_id
            ))

            db.commit()

            flash(
                "Course added successfully.",
                "success"
            )

            return redirect("/view_courses")

        except Exception:

            db.rollback()

            flash(
                "Unable to add course.",
                "danger"
            )

    cursor.execute("""
        SELECT
            instructor_id,
            instructor_name
        FROM instructors
        ORDER BY instructor_name
    """)

    instructors = cursor.fetchall()

    return render_template(
        "add_course.html",
        instructors=instructors
    )


# =========================================================
# VIEW COURSES
# =========================================================

@app.route("/view_courses")
def view_courses():

    if not admin_required():
        return redirect("/admin_login")

    cursor.execute("""
        SELECT
            c.course_id,
            c.course_code,
            c.course_name,
            c.credits,
            c.department,
            c.capacity,
            c.available_seats,
            c.instructor_id,
            i.instructor_name
        FROM courses c
        LEFT JOIN instructors i
        ON c.instructor_id=i.instructor_id
        ORDER BY c.course_id
    """)

    courses = cursor.fetchall()

    return render_template(
        "view_courses.html",
        courses=courses
    )


# =========================================================
# EDIT COURSE
# =========================================================

@app.route(
    "/edit_course/<int:course_id>",
    methods=["GET", "POST"]
)
def edit_course(course_id):

    if not admin_required():
        return redirect("/admin_login")

    if request.method == "POST":

        course_code = request.form["course_code"]
        course_name = request.form["course_name"]
        credits = request.form["credits"]
        department = request.form["department"]
        capacity = request.form["capacity"]
        available_seats = request.form["available_seats"]
        instructor_id = request.form["instructor_id"]

        cursor.execute("""
            UPDATE courses
            SET
                course_code=%s,
                course_name=%s,
                credits=%s,
                department=%s,
                capacity=%s,
                available_seats=%s,
                instructor_id=%s
            WHERE course_id=%s
        """, (
            course_code,
            course_name,
            credits,
            department,
            capacity,
            available_seats,
            instructor_id,
            course_id
        ))

        db.commit()

        flash(
            "Course updated successfully.",
            "success"
        )

        return redirect("/view_courses")

    cursor.execute("""
        SELECT *
        FROM courses
        WHERE course_id=%s
    """, (course_id,))

    course = cursor.fetchone()

    cursor.execute("""
        SELECT
            instructor_id,
            instructor_name
        FROM instructors
        ORDER BY instructor_name
    """)

    instructors = cursor.fetchall()

    return render_template(
        "edit_course.html",
        course=course,
        instructors=instructors
    )


# =========================================================
# DELETE COURSE
# =========================================================

@app.route("/delete_course/<int:course_id>")
def delete_course(course_id):

    if not admin_required():
        return redirect("/admin_login")

    cursor.execute("""
        SELECT COUNT(*)
        FROM enrollments
        WHERE course_id=%s
    """, (course_id,))

    enrollment_count = cursor.fetchone()[0]

    cursor.execute("""
        SELECT COUNT(*)
        FROM waitlist
        WHERE course_id=%s
    """, (course_id,))

    waitlist_count = cursor.fetchone()[0]

    if enrollment_count > 0 or waitlist_count > 0:

        return render_template(
            "course_delete_error.html",
            enrollment_count=enrollment_count,
            waitlist_count=waitlist_count
        )

    cursor.execute("""
        DELETE FROM prerequisites
        WHERE course_id=%s
        OR prerequisite_course_id=%s
    """, (
        course_id,
        course_id
    ))

    cursor.execute("""
        DELETE FROM courses
        WHERE course_id=%s
    """, (course_id,))

    db.commit()

    flash(
        "Course deleted successfully.",
        "warning"
    )

    return redirect("/view_courses")


# =========================================================
# ADD INSTRUCTOR
# =========================================================

@app.route(
    "/add_instructor",
    methods=["GET", "POST"]
)
def add_instructor():

    if not admin_required():
        return redirect("/admin_login")

    if request.method == "POST":

        instructor_name = request.form["instructor_name"]
        email = request.form["email"]
        password = request.form["password"]

        try:

            cursor.execute("""
                INSERT INTO instructors
                (
                    instructor_name,
                    email,
                    password
                )
                VALUES
                (%s,%s,%s)
            """, (
                instructor_name,
                email,
                password
            ))

            db.commit()

            flash(
                "Instructor added successfully.",
                "success"
            )

            return redirect("/view_instructors")

        except Exception:

            db.rollback()

            flash(
                "Unable to add instructor.",
                "danger"
            )

    return render_template(
        "add_instructor.html"
    )


# =========================================================
# VIEW INSTRUCTORS
# =========================================================

@app.route("/view_instructors")
def view_instructors():

    if not admin_required():
        return redirect("/admin_login")

    cursor.execute("""
        SELECT
            i.instructor_id,
            i.instructor_name,
            i.email,
            COUNT(c.course_id)
        FROM instructors i
        LEFT JOIN courses c
        ON i.instructor_id=c.instructor_id
        GROUP BY
            i.instructor_id,
            i.instructor_name,
            i.email
        ORDER BY i.instructor_id
    """)

    instructors = cursor.fetchall()

    return render_template(
        "view_instructors.html",
        instructors=instructors
    )


# =========================================================
# DELETE INSTRUCTOR
# =========================================================

@app.route(
    "/delete_instructor/<int:instructor_id>"
)
def delete_instructor(instructor_id):

    if not admin_required():
        return redirect("/admin_login")

    cursor.execute("""
        SELECT COUNT(*)
        FROM courses
        WHERE instructor_id=%s
    """, (instructor_id,))

    course_count = cursor.fetchone()[0]

    if course_count > 0:

        flash(
            "Cannot delete instructor because courses are assigned to this instructor.",
            "danger"
        )

        return redirect("/view_instructors")

    cursor.execute("""
        DELETE FROM instructors
        WHERE instructor_id=%s
    """, (instructor_id,))

    db.commit()

    flash(
        "Instructor deleted successfully.",
        "warning"
    )

    return redirect("/view_instructors")


# =========================================================
# WAITLIST - ADMIN
# =========================================================

@app.route("/waitlist")
def waitlist():

    if not admin_required():
        return redirect("/admin_login")

    cursor.execute("""
        SELECT
            w.waitlist_id,
            c.course_name,
            s.full_name,
            s.registration_no,
            w.position_no
        FROM waitlist w
        JOIN students s
        ON w.student_id=s.student_id
        JOIN courses c
        ON w.course_id=c.course_id
        ORDER BY
            c.course_name,
            w.position_no
    """)

    waitlist_data = cursor.fetchall()

    return render_template(
        "waitlist.html",
        waitlist=waitlist_data
    )


# =========================================================
# REPORTS
# =========================================================

@app.route("/reports")
def reports():

    if not admin_required():
        return redirect("/admin_login")

    cursor.execute(
        "SELECT COUNT(*) FROM students"
    )
    total_students = cursor.fetchone()[0]

    cursor.execute(
        "SELECT COUNT(*) FROM instructors"
    )
    total_instructors = cursor.fetchone()[0]

    cursor.execute(
        "SELECT COUNT(*) FROM courses"
    )
    total_courses = cursor.fetchone()[0]

    cursor.execute(
        "SELECT COUNT(*) FROM enrollments"
    )
    total_requests = cursor.fetchone()[0]

    cursor.execute("""
        SELECT COUNT(*)
        FROM enrollments
        WHERE status='Approved'
    """)
    approved_requests = cursor.fetchone()[0]

    cursor.execute("""
        SELECT COUNT(*)
        FROM enrollments
        WHERE status='Pending'
    """)
    pending_requests = cursor.fetchone()[0]

    cursor.execute("""
        SELECT COUNT(*)
        FROM enrollments
        WHERE status='Rejected'
    """)
    rejected_requests = cursor.fetchone()[0]

    return render_template(
        "reports.html",
        total_students=total_students,
        total_instructors=total_instructors,
        total_courses=total_courses,
        total_requests=total_requests,
        approved_requests=approved_requests,
        pending_requests=pending_requests,
        rejected_requests=rejected_requests
    )


# =========================================================
# LOGOUT
# =========================================================

@app.route("/logout")
def logout():

    session.clear()

    flash(
        "You have been logged out successfully.",
        "success"
    )

    return redirect("/")


# =========================================================
# 404
# =========================================================

@app.errorhandler(404)
def page_not_found(error):

    return render_template(
        "404.html"
    ), 404


# =========================================================
# 500
# =========================================================

@app.errorhandler(500)
def internal_error(error):

    try:
        db.rollback()
    except Exception:
        pass

    return render_template(
        "404.html"
    ), 500


# =========================================================
# RUN
# =========================================================

if __name__ == "__main__":
    app.run(debug=True)