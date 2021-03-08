/*
Written by hkaase, for use in DIY homebrew tuning applications as an alternative to paid tools. 
It's not great, but it's better than nothing.

TODO:

Create cell class to simplify mess of arrays
Combinatory interpolation using horizontal and vertical values - DONE
Advanced interpolation algorithms (we can be smarter - VE tables do not scale linearly) - DONE. Cell interpolation, both in analysis and after, is complete.
SDL based graph GUI
Convert to QT GUI (someday)

*/
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <vector>
#include "rlutil.h"

using namespace std;

const double AFR_TARGET = 14.7;
const double LAMBDA_TARGET = 1.0;
const double veBoundary = .8;

//For printing colors to console
const int BLACK = 0;
const int BLUE = 1;
const int GREEN = 2;
const int CYAN = 3;
const int RED = 4;
const int MAGENTA = 5;
const int BROWN = 6;
const int GREY = 7;
const int LIGHT_GREY = 8;
const int LIGHT_BLUE = 9;
const int LIGHT_GREEN = 10;
const int LIGHT_CYAN = 11;
const int LIGHT_RED = 12;
const int LIGHT_MAGENTA = 13;
const int YELLOW = 14;
const int WHITE = 15;

int calculateRow(double, double, double);
int calculateCol(double, double, double);
double smoothData(double, double, double);



int main() {
    bool peModeTuning;
    cout << "Do you want support for PE mode tuning? Type y for yes, n for no. (Know that if this is disabled, tuning in the PE range can produce catastrophic results!)" << endl;
    char input;
    cin >> input;
    
    if (input == 'y' || input == 'Y') {
        peModeTuning = true;
        rlutil::setColor(GREEN);
        cout << "Using PE mode" << endl;
    }
    else {
        rlutil::setColor(RED);
        cout << "Not using PE mode" << endl;
    }
    rlutil::setColor(WHITE);
    cout << "Would you like to use default settings for an LS engine? Type y for yes, anything else for no." << endl;
    cin >> input;
    bool useDefaults;
    if (input == 'y' || input == 'Y') {
        useDefaults = true;
    }
    else {
        useDefaults = false;
    }
    int numRows = 21;
    int numCols = 20;
    int minRowVal = 400;
    int maxRowVal = 8000;
    int minColVal = 15;
    int maxColVal = 105;

    if (!useDefaults) {
        cout << "Give me some info about your VE table. This assumes that vacuum is the y-axis, RPM is the x-axis. If you don't know, the values in parentheses should work for an LS. " << endl;
    
        cout << "How many rows? (x-axis, probably 20)" << endl;
        cin >> numRows;
        numRows += 1;
        cout << "How many columns? (y-axis, probably 19)" << endl;
        cin >> numCols;
        numCols += 1;
        
        cout << "What's the minimum value of the rows? (400)" << endl;
        cin >> minRowVal;
        
        cout << "What's the maximum value of the rows? (8000)" << endl;
        cin >> maxRowVal;
        
        cout << "What's the minimum value of the columns? (15)" << endl;
        cin >> minColVal;
        
        cout << "What's the maximum value of the columns? (105)" << endl;
        cin >> maxColVal;
    }
    
    cout << "Generating VE array." << endl;
    double mainVEObserved[numCols + 1][numRows + 1] = {{0}};
    double mainVECorrectionFactor[numCols + 1][numRows + 1] = {{0}};
    double mainVEInput[numCols + 1][numRows + 1] = {{0}};
    double mainVECorrected[numCols + 1][numRows + 1] = {{0}};
    bool isPE[numCols+1][numRows+1] = {{0}};
    double recordCounterVE[numCols + 1][numRows + 1] = {{0}};
    cout << "I have calculated that your row headings look something like this:" << endl;
    
    /*For some unknown reason, the VE tables are not precisely
    approximated to even increments in the headings. Is this 
    the XDF prettying things up for me or is this intentional?
    To fix this, I round the increment of the row up to the nearest
    hundreds place, and the column to the nearest ones place.
    */
    
    int rowIncrement = (maxRowVal - minRowVal) / numRows;
    rowIncrement = (rowIncrement + 50) / 100 * 100;
    int rowValueAtCell = minRowVal;
    int counter = 1;
    rlutil::setColor(BLUE);
    while (rowValueAtCell <= (maxRowVal + 1)) {
        cout << rowValueAtCell << " ";
        mainVEObserved[counter][0] = rowValueAtCell;
        mainVECorrectionFactor[counter][0] = rowValueAtCell;
        recordCounterVE[counter][0] = rowValueAtCell;
        mainVEInput[counter][0] = rowValueAtCell;
        counter++;
        rowValueAtCell += rowIncrement;
    }
    rlutil::setColor(WHITE);
    cout << endl;
    
    
    
    
    cout << "I have calculated that your column headings look something like this:" << endl;
    double colIncrement = (maxColVal - minColVal) / static_cast<double>(numCols);
    colIncrement = ceil(colIncrement);
    int colValueAtCell = minColVal;
    counter = 1;
    rlutil::setColor(BLUE);
    while (colValueAtCell <= (maxColVal)&& counter < numRows) {
        cout << colValueAtCell << endl;
        mainVEObserved[0][counter] = colValueAtCell;
        mainVECorrectionFactor[0][counter] = colValueAtCell;
        recordCounterVE[0][counter] = colValueAtCell;
        mainVEInput[0][counter] = colValueAtCell;
        mainVECorrected[0][counter] = colValueAtCell;
        counter++;
        colValueAtCell += colIncrement;
    }
    rlutil::setColor(WHITE);
    
    //This loop is necessary to ensure the arrays are indeed empty. For whatever reason, garbage data keeps making its way into them.
    for (int i = 1; i <= numCols; i++) {
        for (int j = 1; j <= numRows; j++) {
            mainVEObserved[i][j] = 0;
            recordCounterVE[i][j] = 0;
            isPE[i][j] = 0;
        }
    }
    
    cout << "Are you using AFR or lambda? Type 1 for AFR, 2 for lambda." << endl;
    int choice;
    cin >> choice;
    double target;
    if (choice = 1) {
        rlutil::setColor(GREEN);
        target = AFR_TARGET;
        cout << "Using AFR" << endl;
    }
    else if (choice == 2) {
        rlutil::setColor(GREEN);
        target = LAMBDA_TARGET;
        cout << "Using lambda." << endl;
    }
    else {
        rlutil::setColor(RED);
        cout << "Please choose 1 or 2." << endl;
    }
    rlutil::setColor(WHITE);
    bool deinterpolationEnable;
    
    cout << "Would you like to use deinterpolation? This has the potential to give much more accurate results, however, it is relatively untested and has no sanity checks. Type y for yes, and n for no." << endl;
    cin >> input;
    if (input == 'y' || input == 'Y') {
        deinterpolationEnable = true;
    }
    
    //Input File Processing
    cout << "Please enter name of input log file with the extension. Make sure that it is in the same folder as this executable and that it follws the format of RPM, MAP, and then AFR. If you have PE tuning enabled, it should be RPM, MAP, AFR, then TPS%. Ensure this value is a percentage!" << endl; 
    ifstream logFile;
    string fileName;
    cin >> fileName;
    logFile.open(fileName.c_str());
    cin.ignore(1);
    if (!logFile) {
        rlutil::setColor(RED);
        cout << "Error opening file. Abort" << endl;
        rlutil::setColor(WHITE);
        exit(0);
    }
    
    
    double observedRPM, observedMAP, observedAFR, observedTPS, locationRow1, locationCol1, locationRow2, locationCol2;
    int recordCounter = 0;
    int currentRow, currentCol;
    char trash;
    double minRecords;
    
    cout << "What is the minimum number of records to accept a cell as valid?" << endl;
    cin >> minRecords;
    //If PE not enabled
    if (!peModeTuning) {
        while (logFile >> observedRPM >> trash >> 
        observedMAP >> trash >> observedAFR) {
            if (deinterpolationEnable && (observedRPM > 0)) {
                //Necessary to make the minRecords work with the way interpolation stores counts
                if (recordCounter == 0) {
                    minRecords = minRecords / 20;
                }
                currentRow = (calculateRow(minRowVal, rowIncrement, observedRPM));
                currentCol = (calculateCol(minColVal, colIncrement, observedMAP));
                
                //LS PCM uses interpolation to find where it is between cells
                //Need to reverse interpolation
                //Calculate where we are between cells
                
                locationRow1 = (mainVEObserved[currentRow][0] - observedRPM) /rowIncrement;
                if (locationRow1>= .5) {
                    locationRow2 = locationRow1;
                    locationRow1 = 1 - locationRow1;
                }
                else {
                    locationRow2 = 1 - locationRow1;
                }
                
                locationCol1 = ((mainVEObserved[0][currentCol]) - observedMAP) / colIncrement;
                if (locationCol1 >= .5) {
                    locationCol2 = locationCol1;
                    locationCol1 = 1 - locationCol1;
                }
                else {
                    locationCol2 = 1 - locationCol1;
                }
                mainVEObserved[currentRow][currentCol] += (observedAFR * locationRow1 * locationCol1);
                recordCounterVE[currentRow][currentCol] += (locationRow1*locationCol1);
                
                mainVEObserved[currentRow + 1][currentCol] += (observedAFR * locationRow2 * locationCol1);
                recordCounterVE[currentRow + 1][currentCol] += (locationRow2*locationCol1);
                
                mainVEObserved[currentRow][currentCol + 1] += (observedAFR * locationRow1 * locationCol2);
                recordCounterVE[currentRow][currentCol + 1] += (locationRow1*locationCol2);
                
                mainVEObserved[currentRow + 1][currentCol + 1] += (observedAFR* locationRow2 * locationCol2);
                recordCounterVE[currentRow + 1][currentCol + 1] += (locationRow2*locationCol2);
            }
            else if (!deinterpolationEnable && (observedRPM > 0)){
                currentRow = (calculateRow(minRowVal, rowIncrement, observedRPM));
                currentCol = (calculateCol(minColVal, colIncrement, observedMAP));
                mainVEObserved[currentRow][currentCol] += observedAFR;
                recordCounterVE[currentRow][currentCol] += 1;
            }
            recordCounter++;
        }
        cout << "PE mode DISABLED! Be careful!" << endl;
        cout << "Processed " << recordCounter << " records." << endl;
        
        //If the number of observed records is greater than the established minimum, update the observed AFR in the table.
        for (int i = 1; i < numCols; i++) {
            for (int j = 1; j < numRows; j++) {
                if (recordCounterVE[i][j] >= minRecords) {
                    mainVEObserved[i][j] = mainVEObserved[i][j] / recordCounterVE[i][j];
                }
            }
        }
        
        //If the number of observed records is greater than the minimum, divide the observed AFR by our target AFR.
        for (int i = 1; i < numCols; i++) {
            for (int j = 1; j < numRows; j++) {
                if (recordCounterVE[i][j] >= minRecords) {
                    mainVECorrectionFactor[i][j] = (mainVEObserved[i][j] / target);
                }
            }
        }
    }
    
    //If PE enabled
    else {
        //This is a vector, to ensure ease of modification and adaptability if necessary.
        //Edit this with the correct values for your non-LS application if necessary.
        
        vector <int> tpsRPMArray = {0, 400, 800, 1200, 1600, 2000, 2400, 2800, 3200, 3600, 4000, 4400, 4800, 5200, 5600, 6000, 6400, 6800, 7200};
        
        vector <int> tpsEnableArray;
        cout << "PE mode ENABLED! I need a bit more info now." << endl;
        double eqRatio;
        cout << "What is your commanded PE EQ ratio? (This is making the assumption that it is constant, which in most cases is adequate.)" << endl;
        cin >> eqRatio;
        int peMAPEnable, tpsPercentage;
        cout << "On LS engines, regardless of year, the tables for the TPS percentage required to enable power enrichment are thankfully the same.";
        cout << " This means that I can prompt you for what you have in your tune without knowing specifics about your year model. If you have a non-LS engine, you'll need to do some editing in the main.cpp file." << endl;
        cout << "What is your PE enable MAP? (The unit doesn't matter, as long as it is the same as your log)" << endl;
        cin >> peMAPEnable;
        cout << "Please copy and paste your PE Enable TPS table into this window. When finished, enter an alphabetic character and hit enter." << endl;
        while (cin >> tpsPercentage) {
            tpsEnableArray.push_back(tpsPercentage);
        }
        
        //Read from logfile parameters, in this order.
        while (logFile >> observedRPM >> trash >> 
        observedMAP >> trash >> observedAFR >> trash >> observedTPS) {
            if (deinterpolationEnable && observedRPM > 0) {
                //Necessary to make the minRecords work with the way interpolation stores counts
                if (recordCounter == 0) {
                    minRecords = minRecords / 20;
                }
                currentRow = (calculateRow(minRowVal, rowIncrement, observedRPM));
                currentCol = (calculateCol(minColVal, colIncrement, observedMAP));
                
                //LS PCM uses interpolation to find where it is between cells
                //Need to reverse interpolation
                //Calculate where we are between cells
                
                locationRow1 = (mainVEObserved[currentRow][0] - observedRPM) /rowIncrement;
                if (locationRow1>= .5) {
                    locationRow2 = locationRow1;
                    locationRow1 = 1 - locationRow1;
                }
                else {
                    locationRow2 = 1 - locationRow1;
                }
                
                locationCol1 = ((mainVEObserved[0][currentCol]) - observedMAP) / colIncrement;
                if (locationCol1 >= .5) {
                    locationCol2 = locationCol1;
                    locationCol1 = 1 - locationCol1;
                }
                else {
                    locationCol2 = 1 - locationCol1;
                }
                mainVEObserved[currentRow][currentCol] += (observedAFR * locationRow1 * locationCol1);
                recordCounterVE[currentRow][currentCol] += (locationRow1*locationCol1);
                
                mainVEObserved[currentRow + 1][currentCol] += (observedAFR * locationRow2 * locationCol1);
                recordCounterVE[currentRow + 1][currentCol] += (locationRow2*locationCol1);
                
                mainVEObserved[currentRow][currentCol + 1] += (observedAFR * locationRow1 * locationCol2);
                recordCounterVE[currentRow][currentCol + 1] += (locationRow1*locationCol2);
                
                mainVEObserved[currentRow + 1][currentCol + 1] += (observedAFR* locationRow2 * locationCol2);
                recordCounterVE[currentRow + 1][currentCol + 1] += (locationRow2*locationCol2);
                int i = 0;
                while (tpsRPMArray.at(i) < observedRPM && (i < tpsEnableArray.size())) {
                    i++;
                }
                //This is a dumb way to do this, but essentially the subtraction here is necessary because in the above
                //line we determine the array value AFTER our criteria. The subtraction is a dumb hack to make it work.
                i -= 1;
                //Compare the TPS observed to our array at the RPM range. If it is greater, PE is enabled.
                if (tpsEnableArray.at(i) <= observedTPS) {
                    isPE[currentRow][currentCol] = true;
                }
            }
            else if (!deinterpolationEnable && observedRPM > 0){
                currentRow = (calculateRow(minRowVal, rowIncrement, observedRPM));
                currentCol = (calculateCol(minColVal, colIncrement, observedMAP));
                mainVEObserved[currentRow][currentCol] += observedAFR;
                recordCounterVE[currentRow][currentCol] += 1;
                recordCounter++;
                    
                int i = 0;
                while (tpsRPMArray.at(i) < observedRPM && (i < tpsEnableArray.size())) {
                    i++;
                }
                //This is a dumb way to do this, but essentially the subtraction here is necessary because in the above
                //line we determine the array value AFTER our criteria. The subtraction is a dumb hack to make it work.
                i -= 1;
                    
                //Compare the TPS observed to our array at the RPM range. If it is greater, PE is enabled.
                if (tpsEnableArray.at(i) <= observedTPS) {
                    isPE[currentRow][currentCol] = true;
                }
            }
            recordCounter++;
        }
        
        //If the number of observed records is greater than the established minimum, update the observed AFR in the table.
        for (int i = 1; i < numCols; i++) {
            for (int j = 1; j < numRows; j++) {
                if (recordCounterVE[i][j] >= minRecords) {
                    mainVEObserved[i][j] = mainVEObserved[i][j] / recordCounterVE[i][j];
                }
            }
        }
        
        //If the number of observed records is greater than the minimum, divide the observed AFR by our target AFR.
        for (int i = 1; i < numCols; i++) {
            for (int j = 1; j < numRows; j++) {
                if ((recordCounterVE[i][j] >= minRecords) && !(isPE[i][j])) {
                    mainVECorrectionFactor[i][j] = (mainVEObserved[i][j] / target);
                }
                else if ((recordCounterVE[i][j] >= minRecords) && (isPE[i][j])) {
                    mainVECorrectionFactor[i][j] = (mainVEObserved[i][j] / (target * (1/eqRatio)));
                }
            }
        }
        
        cout << "Processed " << recordCounter << " records." << endl;
    }
    

    
    //Print debug routines
    //Print cells where PE was observed.
    cout << "Power enrichment was observed in the following cells." << endl;
    for (int i = 1; i < numCols; i++) {
           for (int j = 1; j < numRows; j++) {
            if (isPE[i][j]) {
                rlutil::setColor(GREEN);
            }
            cout << isPE[i][j] << " ";
            rlutil::setColor(WHITE);
        }
    cout << endl;
    }
    //VE Observed
    cout << "This is the observed AFR." << endl << endl;
    for (int i = 0; i < numCols; i++) {
        for (int j = 0; j < numRows; j++) {
            if ((i == 0) || (j == 0)) {
                rlutil::setColor(BLUE);
                cout << setw(8) << mainVEObserved[j][i] << " ";
            }
            else if (recordCounterVE[j][i] >= minRecords) {
                if (mainVEObserved[j][i] >= target * 1.05) {
                    rlutil::setColor(RED);
                }
                else if (mainVEObserved[j][i] >= target * 1.01) {
                    rlutil::setColor(LIGHT_RED);
                }
                else if (mainVEObserved[j][i] >= target) {
                    rlutil::setColor(WHITE);
                }
                else if (mainVEObserved[j][i] <= target * .95) {
                    rlutil::setColor(GREEN);
                }
                else if (mainVEObserved[j][i] <= target * .99) {
                    rlutil::setColor(LIGHT_GREEN);
                }
                else if (mainVEObserved[j][i] <= target) {
                    rlutil::setColor(WHITE);
                }
                cout << setw(8) << mainVEObserved[j][i] << " ";
            }
            else {
                rlutil::setColor(GREY);
                cout << setw(8) << "0" << " ";
            }
        }
        cout << endl;
    }
    cout << endl;
    
    //VE Correction Factor
    cout << "This is the proposed VE correction factor." << endl << endl;
    for (int i = 0; i < numCols; i++) {
            for (int j = 0; j < numRows; j++) {
               if ((i == 0) || (j == 0)) {
                    rlutil::setColor(BLUE);
                    cout << setw(8) << mainVECorrectionFactor[j][i] << " ";
                }
                else if (recordCounterVE[j][i] >= minRecords) {
                    if (mainVECorrectionFactor[j][i] >= 1.05) {
                        rlutil::setColor(RED);
                    }
                    else if (mainVECorrectionFactor[j][i] >= 1.01) {
                        rlutil::setColor(LIGHT_RED);
                    }
                    else if (mainVECorrectionFactor[j][i] >= target) {
                        rlutil::setColor(WHITE);
                    }
                    else if (mainVECorrectionFactor[j][i] <= target * .95) {
                        rlutil::setColor(GREEN);
                    }
                    else if (mainVECorrectionFactor[j][i] <= target * .99) {
                        rlutil::setColor(LIGHT_GREEN);
                    }
                    else if (mainVECorrectionFactor[j][i] <= target) {
                        rlutil::setColor(WHITE);
                    }
                    cout << setw(8) << mainVECorrectionFactor[j][i] << " ";
                }
                else {
                    rlutil::setColor(GREY);
                    cout << setw(8) << "0" << " ";
                } 
        }
        cout << endl;
    }
    cout << endl;
    
    //VE Cell Counts
    cout << "This is the number of hits per cell." << endl << endl;
    for (int i = 0; i < numCols; i++) {
        for (int j = 0; j < numRows; j++) {
            if (i == 0 || j == 0) {
                rlutil::setColor(BLUE);
            }
            else if (recordCounterVE[j][i] >= minRecords) {
                rlutil::setColor(GREEN);
            }
            else if (recordCounterVE[j][i] == 0) {
                rlutil::setColor(GREY);
            }
            else {
                rlutil::setColor(RED);
            }
            
            cout << setw(4) << setprecision(4) << recordCounterVE[j][i] << " ";
        }
        cout << endl;
    }
    cout << endl;
    
    cout << "If you would like to use the tuning functionality, please paste your VE table. After your paste, type an alphabetic character and hit enter to exit input. Unfortunately, due to limitations with TunerPro and the tab character in the cmd window, if you use TunerPro you must first convert the tabs to spaces in order for this program to be able to read it." << endl;
    int rowCounter = 1;
    int colCounter = 1;
    counter = 1;
    double inputVE;
    cin.clear();
    cin.ignore(1);

    //VE table input routine.
    while (cin >> inputVE) {
        mainVEInput[rowCounter][colCounter] = inputVE;
        rowCounter++;
        //If the rowCounter is equal to the numRows, this means we reached the end. 
        if (rowCounter == numRows) {
            rowCounter = 1;
            colCounter++;
        }
    }
    
    cout << "This is what you input. Please make sure it is correct!" << endl << endl;
    
    
    //Print the input back to the screen, just to be safe.
    for (int i = 0; i < numCols; i++) {
        for (int j = 0; j < numRows; j++) {
            if (i == 0 || j == 0) {
                rlutil::setColor(BLUE);
            }
            else {
                rlutil::setColor(WHITE);
            }
            cout << setw(8) << mainVEInput[j][i] << " ";
        }
        cout << endl;
    }
    cout << "If this is incorrect, type q to quit. Otherwise, type any character and hit enter to continue." << endl;
    for (int i = 0; i < numCols; i++) {
        for (int j = 0; j < numRows; j++) {
            mainVECorrected[j][i] = mainVEInput[j][i];
        }
    }
    cin.clear();
    cin.ignore(1);
    cin >> input;
    if (input == 'q') {
        exit(0);
    }
    cout << "Okay, I am going to apply the correction factor to the VE you pasted. If there are less than your specified minimum number of cells, nothing happens." << endl;
    
    //Once again, if the observed records was greater than the defined minimum, we do something. In this case, it is apply the correction factor
    //to the VE table.
    for (int i = 1; i < numCols; i++) {
        for (int j = 1; j < numRows; j++) {
            if (recordCounterVE[j][i] >= minRecords) {
                cout << "I corrected the cell at " << j << " by " << i << endl;
                cout << "The factor was " << mainVECorrectionFactor[j][i] << endl;
                mainVECorrected[j][i] = mainVEInput[j][i] * mainVECorrectionFactor[j][i];
            }
        }
    }
    
    //Interpolation (work in progress)
    cout << "Would you like to apply interpolation to the table? This can help smooth out missed cells. Type y for yes, anything else for no." << endl;
    cin >> input;
    cin.ignore(1);
    if (input == 'y') {
        for (int i = 2; i < numCols - 1; i++) {
            for (int j = 2; j < numRows; j++) {
                
                //Horizontal interpolation
                if ((recordCounterVE[j][i] < minRecords) && ((recordCounterVE[j][i-1] >= minRecords) && (recordCounterVE[j][i+1] >= minRecords))) {
                    cout << "Interpolation applied at " << j << " by " << i << endl;
                    mainVEInput[j][i] *= ((mainVECorrectionFactor[j][i-1]) + (mainVECorrectionFactor[j][i+1])) / 2;
                }
                
                //Vertical interpolation
                else if ((recordCounterVE[j][i] < minRecords) && ((recordCounterVE[j-1][i] >= minRecords) && (recordCounterVE[j+1][i] >= minRecords))) {
                    cout << "Interpolation applied at " << j << " by " << i << endl;
                    mainVEInput[j][i] *= ((mainVECorrectionFactor[j-1][i]) + (mainVECorrectionFactor[j+1][i])) / 2;
                }
            }
        }
    }
        
    //Print corrected VE to screen
    cout << "This is the proposed VE corrected table." << endl << endl;
    for (int i = 0; i < numCols; i++) {
        for (int j = 0; j < numRows; j++) {
            if (i == 0 || j == 0) {
                rlutil::setColor(BLUE);
            }
            else if (mainVECorrected[j][i] == mainVEInput[j][i]) {
                rlutil::setColor(WHITE);
            }
            else if (mainVECorrected[j][i] >= mainVEInput[j][i] * 1.05) {
                rlutil::setColor(RED);
            }
            else if (mainVECorrected[j][i] > mainVEInput[j][i]) {
                rlutil::setColor(LIGHT_RED);
            }
            else if (mainVECorrected[j][i] <= mainVEInput[j][i] * .95) {
                rlutil::setColor(GREEN);
            }
            else if (mainVECorrected[j][i] < mainVEInput[j][i]) {
                rlutil::setColor(LIGHT_GREEN);
            }
            cout << setw(8) << mainVECorrected[j][i] << " ";
        }
        cout << endl;
    }
    rlutil::setColor(WHITE);
    
    //Experimental despiking algorithm. Don't use it! 
    
    
    cout << "Would you like to attempt to smooth big outliers? Y for yes, anything else for no. Note that this is most definitely not a perfect smooth, and may make things worse. If uncertain, don't use this." << endl;
    cin >> input;
    if (input == 'y' || input == 'Y') {
        int iterations = 0;
        const double veBoundary = .8;
        while (iterations < 2) {
            for (int j = 3; j < numRows * veBoundary; j++) {
                for (int i = 3; i < numCols * veBoundary; i++) {
                    if (!((mainVECorrected[j][i-2] < mainVECorrected[j][i-1]) &&  (mainVECorrected[j][i-1] < mainVECorrected[j][i]) && (mainVECorrected[j][i] < mainVECorrected[j][i+1]))) {
                        double originalVal, smoothedVal;
                        originalVal = mainVECorrected[j][i];
                        smoothedVal = smoothData(mainVECorrected[j][i-1], mainVECorrected[j][i], mainVECorrected[j][i+1]);
                        if (smoothedVal < originalVal) {
                            cout << "Outlier detected. Outlier was" << mainVECorrected[j][i] << ", data smoothed to " << smoothData(mainVECorrected[j][i-1], mainVECorrected[j][i], mainVECorrected[j][i+1]) << endl;
                            mainVECorrected[j][i] = smoothedVal;
                        }
                    }
                }
            }
            iterations++;
        }
    }
    //Print corrected VE to screen
    cout << "This is the proposed VE corrected table." << endl << endl;
    for (int i = 0; i < numCols; i++) {
        for (int j = 0; j < numRows; j++) {
            if (i == 0 || j == 0) {
                rlutil::setColor(BLUE);
            }
            else if (mainVECorrected[j][i] == mainVEInput[j][i]) {
                rlutil::setColor(WHITE);
            }
            else if (mainVECorrected[j][i] >= mainVEInput[j][i] * 1.05) {
                rlutil::setColor(GREEN);
            }
            else if (mainVECorrected[j][i] > mainVEInput[j][i]) {
                rlutil::setColor(LIGHT_GREEN);
            }
            else if (mainVECorrected[j][i] <= mainVEInput[j][i] * .95) {
                rlutil::setColor(RED);
            }
            else if (mainVECorrected[j][i] < mainVEInput[j][i]) {
                rlutil::setColor(LIGHT_RED);
            }
            cout << setw(8) << mainVECorrected[j][i] << " ";
        }
        cout << endl;
    }
    rlutil::setColor(WHITE);
    
    
    
    
    
    cout << "The corrected table is going to be sent to a file named ve_corrected.txt. However, I need to know what software you're using to output it in a way that you can paste it directly to your tuning software." << endl;
    bool tunerPro, hpTuners;
    cout << "Type 1 for TunerPro, type 2 for HPTuners." << endl;
    cin >> choice;
    if (choice == 1) { 
        tunerPro = true;
        cout << "Using TunerPro compatible output." << endl;
    }
    else {
        hpTuners = true;
        cout << "Using HPTuners compatible output." << endl;
    }
    ofstream outputFile;
    outputFile.open("ve_corrected.txt");
    if (tunerPro) {
        for (int i = 1; i < numCols; i++) {
            for (int j = 1; j < numRows; j++) {
                if (j != 1) {
                    outputFile << '\t';
                }
                outputFile << fixed << setprecision(4) << mainVECorrected[j][i];
            }
            outputFile << '\n';
        }
    }
    else if (hpTuners) {
        for (int i = 1; i < numCols; i++) {
            for (int j = 1; j < numRows; j++) {
                outputFile << fixed << setprecision(4) << mainVECorrected[j][i] << '\t';
            }
            outputFile << '\n';
        }
    }
    return 0;
}


int calculateRow(double minRowVal, double rowIncrement, double observedRPM) {
    int currentRow = 0;
    while ((currentRow * rowIncrement) + minRowVal < observedRPM) {
        currentRow++;
    }
    return currentRow;
}
int calculateCol(double minColVal, double colIncrement, double observedMAP) {
    int currentCol = 0;
    while ((currentCol * colIncrement) + minColVal < observedMAP) {
        currentCol++;
    }
    return currentCol;
}
double smoothData(double smallest, double spike, double biggest) {
    spike = (biggest + smallest) / 2;
    return spike;    
}
