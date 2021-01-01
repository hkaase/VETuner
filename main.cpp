/*
Written by hkaase, for use in DIY homebrew tuning applications as an alternative to paid tools. 
It's not great, but it's better than nothing.

TODO:

Create cell class to simplify mess of arrays
Combinatory interpolation using horizontal and vertical values
Advanced interpolation algorithms (we can be smarter - VE tables do not scale linearly)
Convert to QT GUI (someday)

*/
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <vector>

using namespace std;

const double AFR_TARGET = 14.7;
const double LAMBDA_TARGET = 1.0;

int calculateRow(double, double, double);
int calculateCol(double, double, double);


int main() {
    bool peModeTuning;
    
    cout << "Do you want support for PE mode tuning? Type 1 for yes, 2 for no. (Know that if this is disabled, tuning in the PE range can produce catastrophic results!)" << endl;
    int choice = 0;
    cin >> choice;
    
    if (choice == 1) {
        peModeTuning = true;
        cout << "Bool set to true" << endl;
    }
    else {
        cout << "Bool set to false" << endl;
    }
    
    cout << "Give me some info about your VE table. This assumes that vacuum is the y-axis, RPM is the x-axis. If you don't know, the values in parentheses should work for an LS. " << endl;
    
    cout << "How many rows? (x-axis, probably 20)" << endl;
    int numRows;
    cin >> numRows;
    numRows += 1;
    cout << "How many columns? (y-axis, probably 19)" << endl;
    int numCols;
    cin >> numCols;
    numCols += 1;
    
    cout << "What's the minimum value of the rows? (400)" << endl;
    int minRowVal;
    cin >> minRowVal;
    
    cout << "What's the maximum value of the rows? (8000)" << endl;
    int maxRowVal;
    cin >> maxRowVal;
    
    cout << "What's the minimum value of the columns? (15)" << endl;
    int minColVal;
    cin >> minColVal;
    
    cout << "What's the maximum value of the columns? (105)" << endl;
    int maxColVal;
    cin >> maxColVal;
    
    cout << "Generating VE array." << endl;
    double mainVEObserved[numCols + 1][numRows + 1] = {0};
    double mainVECorrectionFactor[numCols + 1][numRows + 1] = {0};
    double mainVEInput[numCols + 1][numRows + 1] = {0};
    bool isPE[numCols+1][numRows+1] = {0};
    double recordCounterVE[numCols + 1][numRows + 1] = {0};
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
    while (rowValueAtCell <= (maxRowVal + 1)) {
        cout << rowValueAtCell << " ";
        mainVEObserved[counter][0] = rowValueAtCell;
        mainVECorrectionFactor[counter][0] = rowValueAtCell;
        recordCounterVE[counter][0] = rowValueAtCell;
        mainVEInput[counter][0] = rowValueAtCell;
        counter++;
        rowValueAtCell += rowIncrement;
    }
    cout << endl;
    
    
    
    
    cout << "I have calculated that your column headings look something like this:" << endl;
    double colIncrement = (maxColVal - minColVal) / static_cast<double>(numCols);
    colIncrement = ceil(colIncrement);
    int colValueAtCell = minColVal;
    counter = 1;
    while (colValueAtCell <= (maxColVal)&& counter < numRows) {
        cout << colValueAtCell << endl;
        mainVEObserved[0][counter] = colValueAtCell;
        mainVECorrectionFactor[0][counter] = colValueAtCell;
        recordCounterVE[0][counter] = colValueAtCell;
        mainVEInput[0][counter] = colValueAtCell;
        counter++;
        colValueAtCell += colIncrement;
    }
    
    for (int i = 1; i <= numCols; i++) {
        for (int j = 1; j <= numRows; j++) {
            mainVEObserved[i][j] = 0;
            mainVECorrectionFactor[i][j] = 0;
            mainVEInput[i][j] = 0;
            recordCounterVE[i][j] = 0;
            isPE[i][j] = 0;
        }
    }    
    cout << "Are you using AFR or lambda? Type 1 for AFR, 2 for lambda." << endl;
    bool afrEnable = false;
    bool lambdaEnable = false;
    cin >> choice;
    if (choice = 1) {
        afrEnable = true;
        cout << "Using AFR" << endl;
    }
    else if (choice == 2) {
        lambdaEnable = true;
        cout << "Using lambda." << endl;
    }
    else {
        cout << "Please choose 1 or 2." << endl;
    }
    
    //Input File Processing
    cout << "Please enter name of input log file with the extension. Make sure that it is in the same folder as this executable and that it follws the format of RPM, MAP, and then AFR. If you have PE tuning enabled, it should be RPM, MAP, AFR, then TPS%. Ensure this value is a percentage!" << endl; 
    ifstream logFile;
    string fileName;
    cin >> fileName;
    logFile.open(fileName.c_str());
    cin.ignore(1);
    if (!logFile) {
        cout << "Error opening file. Abort" << endl;
        exit(0);
    }
    
    
    double observedRPM, observedMAP, observedAFR, observedTPS;
    int recordCounter = 0;
    int currentRow, currentCol;
    char trash;
    int minRecords;
    
    //If PE not enabled
    if (!peModeTuning) {
        while (logFile >> observedRPM >> trash >> 
        observedMAP >> trash >> observedAFR) {
            currentRow = (calculateRow(minRowVal, rowIncrement, observedRPM));
            currentCol = (calculateCol(minColVal, colIncrement, observedMAP));
            mainVEObserved[currentRow][currentCol] += observedAFR;
            recordCounterVE[currentRow][currentCol] += 1;
            recordCounter++;
            
        }
        cout << "PE mode DISABLED! Be careful!" << endl;
        cout << "Processed " << recordCounter << " records." << endl;
        cout << "What is the minimum number of records to accept a cell as valid?" << endl;
        cin >> minRecords;
        
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
                if (recordCounterVE[i][j] >= minRecords && afrEnable) {
                    mainVECorrectionFactor[i][j] = (mainVEObserved[i][j] / AFR_TARGET);
                }
                if (recordCounterVE[i][j] >= minRecords && lambdaEnable) {
                    mainVECorrectionFactor[i][j] = (mainVEObserved[i][j] / LAMBDA_TARGET);
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
        cout << "What is the minimum number of records to accept a cell as valid?" << endl;
        cin.clear();
        
        //Necessary to clear trash out of buffer for SOME reason.
        cin.ignore(1);
        cin >> minRecords;
        
        //Read from logfile parameters, in this order.
        while (logFile >> observedRPM >> trash >> 
        observedMAP >> trash >> observedAFR >> trash >> observedTPS) {
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
                if ((recordCounterVE[i][j] >= minRecords) && !(isPE[i][j]) && afrEnable) {
                    mainVECorrectionFactor[i][j] = (mainVEObserved[i][j] / AFR_TARGET);
                }
                else if ((recordCounterVE[i][j] >= minRecords) && (isPE[i][j]) && afrEnable) {
                    mainVECorrectionFactor[i][j] = (mainVEObserved[i][j] / (AFR_TARGET * eqRatio));
                }
                
                //If we are using lambda
                else if ((recordCounterVE[i][j] >= minRecords) && !(isPE[i][j]) && lambdaEnable) {
                    mainVECorrectionFactor[i][j] = (mainVEObserved[i][j] / LAMBDA_TARGET);
                }
                else if ((recordCounterVE[i][j] >= minRecords) && (isPE[i][j]) && afrEnable) {
                    mainVECorrectionFactor[i][j] = (mainVEObserved[i][j] / (LAMBDA_TARGET * eqRatio));
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
            cout << isPE[i][j] << " ";
        }
    cout << endl;
    }
    //VE Observed
    cout << "This is the observed AFR." << endl << endl;
    for (int i = 0; i < numCols; i++) {
        for (int j = 0; j < numRows; j++) {
            cout << setw(8) << mainVEObserved[j][i] << " ";
        }
        cout << endl;
    }
    cout << endl;
    
    //VE Correction Factor
    cout << "This is the proposed VE correction factor." << endl << endl;
    for (int i = 0; i < numCols; i++) {
        for (int j = 0; j < numRows; j++) {
            cout << setw(8) << mainVECorrectionFactor[j][i] << " ";
        }
        cout << endl;
    }
    cout << endl;
    
    //VE Cell Counts
    cout << "This is the number of hits per cell." << endl << endl;
    for (int i = 0; i < numCols; i++) {
        for (int j = 0; j < numRows; j++) {
            cout << setw(4) << recordCounterVE[j][i] << " ";
        }
        cout << endl;
    }
    cout << endl;
    
    cout << "If you would like to use the tuning functionality, please paste your VE table. After your paste, type an alphabetic character and hit enter to exit input. Unfortunately, due to limitations with TunerPro and the tab character in the cmd window, if you use TunerPro you must first convert the tabs to spaces in order for this program to be able to read it." << endl;
    int rowCounter = 1;
    int colCounter = 1;
    counter = 1;
    double inputVE;
    
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
            cout << setw(8) << mainVEInput[j][i] << " ";
        }
        cout << endl;
    }
    cout << "If this is incorrect, type q to quit. Otherwise, hit enter to continue." << endl;
    char input;
    cin.clear();
    cin >> trash;
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
                mainVEInput[j][i] *= mainVECorrectionFactor[j][i];
            }
        }
    }
    
    //Interpolation (work in progress)
    cout << "Would you like to apply interpolation to the table? This can help smooth out missed cells. Type y for yes, anything else for no." << endl;
    cin >> input;
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
            cout << setw(8) << mainVEInput[j][i] << " ";
        }
        cout << endl;
    }
    
    
    
    
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
                outputFile << fixed << setprecision(4) << mainVEInput[j][i];
            }
            outputFile << '\n';
        }
    }
    else if (hpTuners) {
        for (int i = 1; i < numCols; i++) {
            for (int j = 1; j < numRows; j++) {
                outputFile << fixed << setprecision(4) << mainVEInput[j][i] << '\t';
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
    return currentRow + 1;
}
int calculateCol(double minColVal, double colIncrement, double observedMAP) {
    int currentCol = 0;
    while ((currentCol * colIncrement) + minColVal < observedMAP) {
        currentCol++;
    }
    return currentCol + 1;
}