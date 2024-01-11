import re
import psycopg2
from datetime import datetime

# PostgreSQL connection parameters
db_params = {
    'dbname': 'your_database',
    'user': 'your_user',
    'password': 'your_password',
    'host': 'your_host',
    'port': 'your_port'
}

# Regular expression for parsing PostgreSQL logs
log_pattern = re.compile(r'(?P<timestamp>.*?)\s.*?FATAL:\s(?P<message>.*?)\n')

# Function to monitor PostgreSQL logs and insert into a PostgreSQL table
def monitor_logs(log_file_path, table_name):
    with open(log_file_path, 'r') as log_file:
        for line in log_file:
            match = log_pattern.match(line)
            if match:
                timestamp_str = match.group('timestamp')
                message = match.group('message')

                # Convert timestamp string to datetime
                timestamp = datetime.strptime(timestamp_str, '%Y-%m-%d %H:%M:%S.%f')

                # Insert into PostgreSQL table
                insert_query = f"INSERT INTO {table_name} (timestamp, message) VALUES (%s, %s)"
                values = (timestamp, message)

                try:
                    with psycopg2.connect(**db_params) as conn:
                        with conn.cursor() as cursor:
                            cursor.execute(insert_query, values)
                        conn.commit()
                except Exception as e:
                    print(f"Error inserting into database: {e}")

if __name__ == "__main__":
    log_file_path = "/path/to/postgresql.log"  # Replace with your PostgreSQL log file path
    table_name = "fatal_events"  # Replace with your PostgreSQL table name

    monitor_logs(log_file_path, table_name)
