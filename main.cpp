#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>

using namespace std;

const double AFR_TARGET = 14.7;
const double LAMBDA_TARGET = 1.0;

int calculateRow(double, double, double);
int calculateCol(double, double, double);


int main() {
    cout << "Give me some info about your VE table. This assumes that vacuum is the y-axis, RPM is the x-axis." << endl;
    
    cout << "How many rows?" << endl;
    int numRows;
    cin >> numRows;
    numRows += 1;
    cout << "How many columns?" << endl;
    int numCols;
    cin >> numCols;
    numCols += 1;
    
    cout << "What's the minimum value of the rows?" << endl;
    int minRowVal;
    cin >> minRowVal;
    
    cout << "What's the maximum value of the rows?" << endl;
    int maxRowVal;
    cin >> maxRowVal;
    
    cout << "What's the minimum value of the columns?" << endl;
    int minColVal;
    cin >> minColVal;
    
    cout << "What's the maximum value of the columns?" << endl;
    int maxColVal;
    cin >> maxColVal;
    
    cout << "Generating VE array." << endl;
    double mainVEObserved[numCols + 1][numRows + 1] = {0};
    double mainVECorrectionFactor[numCols + 1][numRows + 1] = {0};
    double mainVEInput[numCols + 1][numRows + 1] = {0};
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
        }
    }    cout << "Are you using AFR or lambda? Type 1 for AFR, 2 for lambda." << endl;
    bool afrEnable = false;
    bool lambdaEnable = false;
    int choice = -1;
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
    
    cout << "Please enter name of input log file with the extension. Make sure that it is in the same folder as this executable and that it follws the format of RPM, MAP, and then AFR." << endl; 
    ifstream logFile;
    string fileName;
    cin >> fileName;
    logFile.open(fileName.c_str());
    if (!logFile) {
        cout << "Error opening file. Abort" << endl;
        exit(0);
    }
    
    
    double observedRPM, observedMAP, observedAFR;
    int recordCounter = 0;
    int currentRow, currentCol;
    char trash;
    while (logFile >> observedRPM >> trash >> 
    observedMAP >> trash >> observedAFR) {
        //cout << observedRPM << "," << observedMAP << "," << observedAFR << 
        //"," << endl;
        currentRow = (calculateRow(minRowVal, rowIncrement, observedRPM));
        //cout << "Calculated row is " << currentRow << endl;;
        currentCol = (calculateCol(minColVal, colIncrement, observedMAP));
        //cout << "Calculated column is " << currentCol << endl;
        mainVEObserved[currentRow][currentCol] += observedAFR;
        recordCounterVE[currentRow][currentCol] += 1;
        recordCounter++;
    }
    cout << "Processed " << recordCounter << " records." << endl;
    cout << "What is the minimum number of records to accept a cell as valid?" << endl;
    int minRecords;
    cin >> minRecords;
    
    for (int i = 1; i < numCols; i++) {
        for (int j = 1; j < numRows; j++) {
            if (recordCounterVE[i][j] >= minRecords) {
                mainVEObserved[i][j] = mainVEObserved[i][j] / recordCounterVE[i][j];
            }
        }
    }
    for (int i = 1; i < numCols; i++) {
        for (int j = 1; j < numRows; j++) {
            if (recordCounterVE[i][j] >= minRecords) {
                mainVECorrectionFactor[i][j] = (mainVEObserved[i][j] / AFR_TARGET);
            }
        }
    }
    //print debug routines
    
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
    
    cout << "Please paste your VE table. After your paste, type an alphabetic character to exit input." << endl;
    int rowCounter = 1;
    int colCounter = 1;
    counter = 1;
    double inputVE;
    while (cin >> inputVE) {
        mainVEInput[rowCounter][colCounter] = inputVE;
        rowCounter++;
        if (rowCounter == numRows) {
            rowCounter = 1;
            colCounter++;
        }
    }
    
    cout << "This is what you input. Please make sure it is correct!" << endl << endl;
    for (int i = 0; i < numCols; i++) {
        for (int j = 0; j < numRows; j++) {
            cout << setw(8) << mainVEInput[j][i] << " ";
        }
        cout << endl;
    }
    cout << endl;
    cout << "If this is incorrect, type q to quit. Otherwise, hit enter to continue." << endl;
    char input;
    cin.clear();
    cin >> trash;
    cin >> input;
    if (input == 'q') {
        exit(0);
    }
    cout << "Okay, I am going to apply the correction factor to the VE you pasted. If there are less than your specified minimum number of cells, nothing happens." << endl;
    
    for (int i = 1; i <= numCols; i++) {
        for (int j = 1; j < numRows; j++) {
            if (recordCounterVE[j][i] >= minRecords) {
                mainVEInput[j][i] *= mainVECorrectionFactor[j][i];
            }
        }
    }
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