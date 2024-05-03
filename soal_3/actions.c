#include <stdio.h>
#include <string.h>
#include "actions.h"

void gap(float distance, char *output) {
    if (distance < 3.5) sprintf(output, "Gogogo");
    else if (distance >= 3.5 && distance < 10) sprintf(output, "Push");
    else if (distance > 10) sprintf(output, "Stay out of trouble");
    else sprintf(output, "Invalid distance!");
}

void fuel(int fuel_percent, char *output) {
    if (fuel_percent > 80) sprintf(output, "Push Push Push");
    else if (fuel_percent >= 50 && fuel_percent <= 80) sprintf(output, "You can go");
    else if (fuel_percent < 50) sprintf(output, "Conserve Fuel");
}

void tire(int tire_usage, char *output) {
    if (tire_usage > 80) sprintf(output, "Go Push Go Push");
    else if (tire_usage >= 50 && tire_usage <= 80) sprintf(output, "Good Tire Wear");
    else if (tire_usage >= 30 && tire_usage < 50) sprintf(output, "Conserve Your Tire");
    else sprintf(output, "Box Box Box");
}

void tire_change(char* current_tire, char *output) {
    if (strcmp(current_tire, "Soft") == 0) sprintf(output, "Mediums Ready");
    else if (strcmp(current_tire, "Medium") == 0) sprintf(output, "Box for Softs");
    else sprintf(output, "Unknown tire type");
}
