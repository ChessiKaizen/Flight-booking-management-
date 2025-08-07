#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#define MAX_PLANES 10
#define MAX_FLIGHTS 50
#define LARGE_ROWS 10
#define LARGE_COLS 6
#define SMALL_ROWS 6
#define SMALL_COLS 4
#define MAX_AIRLINES 10
#define MAX_COUNTRIES 7
#define DATE_FORMAT_LEN 11 // YYYY-MM-DD + null terminator
#define FLIGHT_DATA_FILE "flights.md"

// --- Data Structures ---

typedef struct {
    char name[30];
} City;

typedef struct {
    char name[30];
    City cities[5];
    int city_count;
} Country;

typedef struct {
    char flight_number[10];
    char origin[30];
    char destination[30];
    char departure_date[DATE_FORMAT_LEN];
    double price;
    int seat_map[LARGE_ROWS][LARGE_COLS];
    int booked_seats;
} FlightInfo;

typedef struct {
    char plane_id[10];
    char size[10];
    int total_seats;
    int total_flights;
    FlightInfo flights[MAX_FLIGHTS];
} Plane;

typedef struct {
    char name[30];
    Plane fleet[MAX_PLANES];
    int plane_count;
} Airline;

// --- Global Variables ---
Country destinations[MAX_COUNTRIES];
int total_countries = 0;
Airline airlines[MAX_AIRLINES];
int total_airlines = 0;

// --- Function Prototypes ---
void setupPlane(Plane *plane, const char *id, const char *size);
void assignFlight(Plane *plane, const char *flight_num, const char *origin, const char *dest, const char *date, double price);
void loadFlightData(const char* filename);
void bookFlight();
void bookingSummary();
void cancelBooking();

// --- Calendar and Date Functions ---

// Returns 1 if leap year, 0 otherwise
int is_leap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Returns the number of days in a given month of a given year
int get_days_in_month(int year, int month) {
    if (month < 1 || month > 12) return 0;
    if (month == 2) {
        return is_leap(year) ? 29 : 28;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }
    return 31;
}

// Returns the day of the week (0=Sun, 1=Mon, ..., 6=Sat) for a given date
int get_day_of_week(int year, int month, int day) {
    struct tm time_in = {0};
    time_in.tm_year = year - 1900;
    time_in.tm_mon = month - 1;
    time_in.tm_mday = day;
    mktime(&time_in);
    return time_in.tm_wday;
}

// Checks if there are any available flights for a specific route and date
int hasFlightsOnDate(const char* date, const char* origin, const char* destination) {
    for (int i = 0; i < total_airlines; i++) {
        for (int j = 0; j < airlines[i].plane_count; j++) {
            Plane* plane = &airlines[i].fleet[j];
            for (int k = 0; k < plane->total_flights; k++) {
                FlightInfo* flight = &plane->flights[k];
                if (strcmp(flight->origin, origin) == 0 &&
                    strcmp(flight->destination, destination) == 0 &&
                    strcmp(flight->departure_date, date) == 0) {
                    if (flight->booked_seats < plane->total_seats) {
                        return 1; // Found an available flight
                    }
                }
            }
        }
    }
    return 0; // No flights found
}

// Displays a monthly calendar, marking dates with available flights
void displayCalendar(int year, int month, const char* origin, const char* destination) {
    const char* month_names[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    printf("\n      %s %d\n", month_names[month - 1], year);
    printf("---------------------------\n");
    printf(" Sun Mon Tue Wed Thu Fri Sat\n");

    int days_in_month = get_days_in_month(year, month);
    int first_day = get_day_of_week(year, month, 1);

    for (int i = 0; i < first_day; i++) {
        printf("    ");
    }

    for (int day = 1; day <= days_in_month; day++) {
        char date_str[DATE_FORMAT_LEN];
        sprintf(date_str, "%04d-%02d-%02d", year, month, day);
        
        if (hasFlightsOnDate(date_str, origin, destination)) {
            printf("%3d*", day); // Mark with a star
        } else {
            printf("%4d", day);
        }

        if ((first_day + day) % 7 == 0) {
            printf("\n");
        }
    }
    printf("\n---------------------------\n");
    printf("(*) Dates with available flights\n");
}

// --- Core Application Logic ---

// Initializes destination data
void initializeDestinations() {
    strcpy(destinations[0].name, "Japan");
    strcpy(destinations[0].cities[0].name, "Tokyo");
    strcpy(destinations[0].cities[1].name, "Osaka");
    destinations[0].city_count = 2;

    strcpy(destinations[1].name, "Thailand");
    strcpy(destinations[1].cities[0].name, "Bangkok");
    destinations[1].city_count = 1;

    strcpy(destinations[2].name, "Cambodia");
    strcpy(destinations[2].cities[0].name, "Phnom Penh");
    strcpy(destinations[2].cities[1].name, "Siem Reap");
    destinations[2].city_count = 2;

    strcpy(destinations[3].name, "Vietnam");
    strcpy(destinations[3].cities[0].name, "Ho Chi Minh City");
    strcpy(destinations[3].cities[1].name, "Hanoi");
    destinations[3].city_count = 2;

    strcpy(destinations[4].name, "Malaysia");
    strcpy(destinations[4].cities[0].name, "Kuala Lumpur");
    strcpy(destinations[4].cities[1].name, "Penang");
    destinations[4].city_count = 2;

    strcpy(destinations[5].name, "Singapore");
    strcpy(destinations[5].cities[0].name, "Singapore City");
    destinations[5].city_count = 1;

    strcpy(destinations[6].name, "South Korea");
    strcpy(destinations[6].cities[0].name, "Seoul");
    strcpy(destinations[6].cities[1].name, "Busan");
    destinations[6].city_count = 2;

    total_countries = 7;
}

// Sets up a plane's details
void setupPlane(Plane *plane, const char *id, const char *size) {
    strcpy(plane->plane_id, id);
    strcpy(plane->size, size);
    plane->total_flights = 0;
    if (strcmp(size, "Large") == 0) {
        plane->total_seats = LARGE_ROWS * LARGE_COLS;
    } else {
        plane->total_seats = SMALL_ROWS * SMALL_COLS;
    }
}

// Assigns flight details to a specific plane
void assignFlight(Plane *plane, const char *flight_num, const char *origin, const char *dest, const char *date, double price) {
    if (plane->total_flights >= MAX_FLIGHTS) return;

    FlightInfo *new_flight = &plane->flights[plane->total_flights];
    strcpy(new_flight->flight_number, flight_num);
    strcpy(new_flight->origin, origin);
    strcpy(new_flight->destination, dest);
    strcpy(new_flight->departure_date, date);
    new_flight->price = price;
    new_flight->booked_seats = 0;
    memset(new_flight->seat_map, 0, sizeof(new_flight->seat_map));
    
    plane->total_flights++;
}

// Loads all flight data from the external file
void loadFlightData(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error: Cannot open flight data file. Make sure 'flights.md' is in the same directory");
        exit(1);
    }

    char line[256];
    // Skip header lines in the markdown file
    for(int i=0; i<5; ++i) {
        if(fgets(line, sizeof(line), file) == NULL) break;
    }

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0; // Remove newline characters
        if (strlen(line) < 10) continue; // Skip empty or short lines

        char* token;
        char parts[8][50];
        int part_count = 0;

        token = strtok(line, ",");
        while (token != NULL && part_count < 8) {
            strcpy(parts[part_count++], token);
            token = strtok(NULL, ",");
        }

        if (part_count != 8) continue; // Skip malformed lines

        char* airline_name = parts[0];
        char* plane_id = parts[1];
        char* plane_size = parts[2];
        char* flight_num = parts[3];
        char* origin = parts[4];
        char* dest = parts[5];
        char* date = parts[6];
        double price = atof(parts[7]);

        Airline* airline = NULL;
        for (int i = 0; i < total_airlines; i++) {
            if (strcmp(airlines[i].name, airline_name) == 0) {
                airline = &airlines[i];
                break;
            }
        }
        if (airline == NULL) {
            if (total_airlines >= MAX_AIRLINES) continue;
            airline = &airlines[total_airlines++];
            strcpy(airline->name, airline_name);
            airline->plane_count = 0;
        }

        Plane* plane = NULL;
        for (int i = 0; i < airline->plane_count; i++) {
            if (strcmp(airline->fleet[i].plane_id, plane_id) == 0) {
                plane = &airline->fleet[i];
                break;
            }
        }
        if (plane == NULL) {
            if (airline->plane_count >= MAX_PLANES) continue;
            plane = &airline->fleet[airline->plane_count++];
            setupPlane(plane, plane_id, plane_size);
        }

        assignFlight(plane, flight_num, origin, dest, date, price);
    }
    fclose(file);
}

// Displays the seat map for a given flight
void displaySeatMap(FlightInfo *flight, const char *plane_size) {
    int rows = (strcmp(plane_size, "Large") == 0) ? LARGE_ROWS : SMALL_ROWS;
    int cols = (strcmp(plane_size, "Large") == 0) ? LARGE_COLS : SMALL_COLS;

    printf("   ");
    for (int c = 0; c < cols; c++) printf(" %c", 'A' + c);
    printf("\n");
    for (int r = 0; r < rows; r++) {
        printf("%2d ", r + 1);
        for (int c = 0; c < cols; c++)
            printf(" %c", flight->seat_map[r][c] ? 'X' : 'O');
        printf("\n");
    }
}

// Prompts user to select a city from a country
int selectCityFromCountry(char *selectedCity, const char *prompt) {
    printf("%s\n", prompt);
    printf("Available countries:\n");
    for (int i = 0; i < total_countries; i++) {
        printf("   %d. %s\n", i + 1, destinations[i].name);
    }

    int countryChoice;
    printf("Select country (number): ");
    if (scanf("%d", &countryChoice) != 1) {
        while (getchar() != '\n');
        return 0;
    }
    while (getchar() != '\n');

    if (countryChoice < 1 || countryChoice > total_countries) return 0;

    Country *selectedCountry = &destinations[countryChoice - 1];
    printf("Available cities in %s:\n", selectedCountry->name);
    for (int j = 0; j < selectedCountry->city_count; j++) {
        printf("   %d. %s\n", j + 1, selectedCountry->cities[j].name);
    }

    int cityChoice;
    printf("Select city (number): ");
    if (scanf("%d", &cityChoice) != 1) {
        while (getchar() != '\n');
        return 0;
    }
    while (getchar() != '\n');

    if (cityChoice < 1 || cityChoice > selectedCountry->city_count) return 0;

    strcpy(selectedCity, selectedCountry->cities[cityChoice - 1].name);
    return 1;
}

// Handles the process of booking seats for a number of passengers
int processSeatBooking(FlightInfo *selectedFlight, Plane *selectedPlane, int numPassengers) {
    int seatsBookedSuccessfully = 0;
    for (int i = 0; i < numPassengers; i++) {
        int rows = strcmp(selectedPlane->size, "Large") == 0 ? LARGE_ROWS : SMALL_ROWS;
        int cols = strcmp(selectedPlane->size, "Large") == 0 ? LARGE_COLS : SMALL_COLS;

        printf("\n--- Booking for Passenger %d of %d ---\n", i + 1, numPassengers);
        displaySeatMap(selectedFlight, selectedPlane->size);

        char seat[5];
        printf("Choose seat for Passenger %d (e.g., 3B): ", i + 1);
        scanf("%s", seat);
        while (getchar() != '\n');

        char colChar = toupper(seat[strlen(seat) - 1]);
        seat[strlen(seat) - 1] = '\0';
        int row = atoi(seat) - 1;
        int col = colChar - 'A';

        if (row < 0 || row >= rows || col < 0 || col >= cols || selectedFlight->seat_map[row][col] == 1) {
            printf("Invalid or already booked seat. Please try again for this passenger.\n");
            i--; // Re-do for this passenger
            continue;
        }

        selectedFlight->seat_map[row][col] = 1;
        selectedFlight->booked_seats++;
        printf("Booking successful for seat %d%c.\n", row + 1, colChar);
        seatsBookedSuccessfully++;
    }
    return seatsBookedSuccessfully;
}

// Main flight booking workflow
void bookFlight() {
    char originCity[30], destinationCity[30], departureDate[DATE_FORMAT_LEN];

    if (!selectCityFromCountry(originCity, "Select your departure city:")) {
        printf("Invalid selection. Aborting.\n");
        return;
    }
    if (!selectCityFromCountry(destinationCity, "Select your destination city:")) {
        printf("Invalid selection. Aborting.\n");
        return;
    }

    // --- Calendar Date Selection ---
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int year = tm.tm_year + 1900;
    int month = tm.tm_mon + 1;
    char choice[10];
    
    printf("\n--- Select Departure Date ---\n");
    while(1) {
        displayCalendar(year, month, originCity, destinationCity);
        printf("Enter a day, 'n' for next month, 'p' for prev month, or 'q' to quit: ");
        scanf("%s", choice);
        while (getchar() != '\n');

        if (strcmp(choice, "n") == 0) {
            month++; if (month > 12) { month = 1; year++; }
        } else if (strcmp(choice, "p") == 0) {
            month--; if (month < 1) { month = 12; year--; }
        } else if (strcmp(choice, "q") == 0) {
            return;
        } else if (isdigit(choice[0])) {
            int day = atoi(choice);
            if (day < 1 || day > get_days_in_month(year, month)) {
                printf("Invalid day.\n");
            } else {
                sprintf(departureDate, "%04d-%02d-%02d", year, month, day);
                if (!hasFlightsOnDate(departureDate, originCity, destinationCity)) {
                    printf("No flights on %s. Pick a date with '*'.\n", departureDate);
                } else {
                    break; // Date selected
                }
            }
        } else {
            printf("Invalid input.\n");
        }
    }

    // --- List Available Flights for Selected Date ---
    printf("\n--- Available Flights from %s to %s on %s ---\n", originCity, destinationCity, departureDate);
    
    typedef struct { Airline *airline; Plane *plane; FlightInfo *flight; } MatchedFlight;
    MatchedFlight matchingFlights[MAX_FLIGHTS];
    int matchCount = 0;

    for (int i = 0; i < total_airlines; i++) {
        for (int j = 0; j < airlines[i].plane_count; j++) {
            Plane *p = &airlines[i].fleet[j];
            for (int k = 0; k < p->total_flights; k++) {
                FlightInfo *f = &p->flights[k];
                if (strcmp(f->origin, originCity) == 0 && strcmp(f->destination, destinationCity) == 0 && strcmp(f->departure_date, departureDate) == 0) {
                    if (f->booked_seats < p->total_seats) {
                        matchingFlights[matchCount++] = (MatchedFlight){&airlines[i], p, f};
                    }
                }
            }
        }
    }

    if (matchCount == 0) {
        printf("An error occurred. No flights found despite calendar marking.\n");
        return;
    }

    printf("%-5s%-15s%-12s%-8s%-10s\n", "#", "Airline", "Flight No.", "Price", "Available");
    for (int i = 0; i < matchCount; i++) {
        FlightInfo *f = matchingFlights[i].flight;
        Plane *p = matchingFlights[i].plane;
        printf("%-5d%-15s%-12s$%-7.2f%-10d\n", i + 1, matchingFlights[i].airline->name, f->flight_number, f->price, p->total_seats - f->booked_seats);
    }

    // --- User Selects Flight and Books Seats ---
    int flightChoice;
    printf("Select flight (number) to book: ");
    if (scanf("%d", &flightChoice) != 1 || flightChoice < 1 || flightChoice > matchCount) {
        printf("Invalid selection.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    MatchedFlight *chosen = &matchingFlights[flightChoice - 1];
    int numPassengers;
    printf("How many passengers? ");
    if (scanf("%d", &numPassengers) != 1 || numPassengers <= 0 || numPassengers > (chosen->plane->total_seats - chosen->flight->booked_seats)) {
        printf("Invalid number of passengers or not enough seats.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    int bookedSeats = processSeatBooking(chosen->flight, chosen->plane, numPassengers);

    printf("\n--- Booking Summary ---\n");
    printf("Successfully booked %d seat(s) on %s flight %s from %s to %s.\n",
            bookedSeats, chosen->airline->name, chosen->flight->flight_number, 
            chosen->flight->origin, chosen->flight->destination);
    printf("Total price: $%.2f\n", chosen->flight->price * bookedSeats);
}

// Displays a summary of all booked seats
void bookingSummary() {
    printf("\n--- Booking Summary (Booked Seats Only) ---\n");
    printf("%-15s%-12s%-20s%-20s%-15s%-8s%-5s\n", "Airline", "Flight No.", "Origin", "Destination", "Date", "Price", "Seat");
    int found_booking = 0;

    for (int i = 0; i < total_airlines; i++) {
        for (int j = 0; j < airlines[i].plane_count; j++) {
            Plane *p = &airlines[i].fleet[j];
            for (int k = 0; k < p->total_flights; k++) {
                FlightInfo *f = &p->flights[k];
                int rows = strcmp(p->size, "Large") == 0 ? LARGE_ROWS : SMALL_ROWS;
                int cols = strcmp(p->size, "Large") == 0 ? LARGE_COLS : SMALL_COLS;
                for (int r = 0; r < rows; r++) {
                    for (int c = 0; c < cols; c++) {
                        if (f->seat_map[r][c] == 1) {
                            printf("%-15s%-12s%-20s%-20s%-15s$%-7.2f%2d%c\n",
                                airlines[i].name, f->flight_number, f->origin, f->destination, f->departure_date, f->price, r + 1, 'A' + c);
                            found_booking = 1;
                        }
                    }
                }
            }
        }
    }
    if (!found_booking) printf("No bookings found.\n");
}

// Cancels a specific booking
void cancelBooking() {
    char flightNum[10];
    printf("Enter Flight Number to cancel (e.g., AA-001F1): ");
    scanf("%s", flightNum);
    while (getchar() != '\n');

    for (int i = 0; i < total_airlines; i++) {
        for (int j = 0; j < airlines[i].plane_count; j++) {
            Plane *p = &airlines[i].fleet[j];
            for (int k = 0; k < p->total_flights; k++) {
                FlightInfo *f = &p->flights[k];
                if (strcmp(f->flight_number, flightNum) == 0) {
                    displaySeatMap(f, p->size);
                    char seat[6];
                    printf("Enter seat to cancel (e.g., 2B): ");
                    scanf("%s", seat);
                    while (getchar() != '\n');

                    char colChar = toupper(seat[strlen(seat) - 1]);
                    seat[strlen(seat) - 1] = '\0';
                    int row = atoi(seat) - 1;
                    int col = colChar - 'A';
                    int rows = strcmp(p->size, "Large") == 0 ? LARGE_ROWS : SMALL_ROWS;
                    int cols = strcmp(p->size, "Large") == 0 ? LARGE_COLS : SMALL_COLS;

                    if (row < 0 || row >= rows || col < 0 || col >= cols || f->seat_map[row][col] == 0) {
                        printf("Error: Seat %d%c is not valid or not booked.\n", row + 1, colChar);
                        return;
                    }
                    
                    f->seat_map[row][col] = 0;
                    f->booked_seats--;
                    printf("Booking for seat %d%c on flight %s canceled.\n", row + 1, colChar, f->flight_number);
                    return;
                }
            }
        }
    }
    printf("Flight Number %s not found.\n", flightNum);
}

// Main function
int main() {
    memset(airlines, 0, sizeof(airlines));
    initializeDestinations();
    loadFlightData(FLIGHT_DATA_FILE);

    char name[100];
    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    int choice;
    while (1) {
        printf("\n===== Airline Booking System =====\n");
        printf("Welcome, %s!\n", name);
        printf("1. Book a Flight\n");
        printf("2. View Booking Summary\n");
        printf("3. Cancel a Booking\n");
        printf("4. Exit\n");
        printf("Choose an option: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        switch (choice) {
            case 1: bookFlight(); break;
            case 2: bookingSummary(); break;
            case 3: cancelBooking(); break;
            case 4: printf("Goodbye, %s!\n", name); return 0;
            default: printf("Invalid option.\n");
        }
    }
    return 0;
}
