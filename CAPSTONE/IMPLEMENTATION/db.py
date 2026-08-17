import mysql.connector

db = mysql.connector.connect(
    host="localhost",
    user="root",
    password="0514",
    database="university_course_db"
)

cursor = db.cursor()