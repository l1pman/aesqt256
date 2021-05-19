#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QClipboard>
#include <QByteArray>

#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*---------------------------------------------------*/

void MainWindow::encrypt(){
    keyExpansion();


    QByteArray plainTextArray(ui->textBeforeField->toPlainText().toLocal8Bit());

    while (plainTextArray.size() * 8 % 128 != 0){
        plainTextArray.push_back('A');
    }

    QByteArray encodedTextArray(plainTextArray);

    int index = 0;
    int amountOfBlocks = (plainTextArray.size() * 8) / 128;
    for(int block = 0; block < amountOfBlocks; block++){

        for(int i = 0; i < 16; i++){
            tempForState[i] = plainTextArray[i + index];
        }


        for (int i = 0; i < Nb; i++) {
            for (int j = 0; j < Nb; j++) {
                state[Nb*i + j] = tempForState[Nb*j + i];
            }
        }


        addRoundKey(0);

        for(int round = 1; round < Nr; round++){
            subBytes();
            shiftRows();
            mixColumns();
            addRoundKey(round);
        }

        subBytes();
        shiftRows();
        addRoundKey(Nr);

            for (int i = 0; i < 4; i++) {
            for (int j = 0; j < Nb; j++) {
                encodedTextArray[i+4*j+index] = state[Nb*i+j];
            }
        }
        index += 16;
    }

    ui->textAftedField->setPlainText(encodedTextArray.toHex(0));
    plainTextArray.clear();
    encodedTextArray.clear();
}

void MainWindow::addRoundKey(int round){
    uint8_t c;

    for (c = 0; c < Nb; c++) {
        state[Nb*0+c] = state[Nb*0+c]^expandedKey[4*Nb*round+4*c+0];
        state[Nb*1+c] = state[Nb*1+c]^expandedKey[4*Nb*round+4*c+1];
        state[Nb*2+c] = state[Nb*2+c]^expandedKey[4*Nb*round+4*c+2];
        state[Nb*3+c] = state[Nb*3+c]^expandedKey[4*Nb*round+4*c+3];
    }
}

void MainWindow::subBytes(){
    uint8_t i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < Nb; j++) {
            state[Nb*i+j] = s_box[state[Nb*i+j]];
        }
    }
}

void MainWindow::shiftRows(){
    uint8_t i, k, s, tmp;

    for (i = 1; i < 4; i++) {
        s = 0;
        while (s < i) {
            tmp = state[Nb*i+0];

            for (k = 1; k < Nb; k++) {
                state[Nb*i+k-1] = state[Nb*i+k];
            }

            state[Nb*i+Nb-1] = tmp;
            s++;
        }
    }
}

void MainWindow::mixColumns(){
    uint8_t a[] = {0x02, 0x01, 0x01, 0x03}; // a(x) = {02} + {01}x + {01}x2 + {03}x3
    uint8_t i, j, col[4], res[4];

    for (j = 0; j < Nb; j++) {
        for (i = 0; i < 4; i++) {
            col[i] = state[Nb*i+j];
        }

        coef_mult(a, col, res);

        for (i = 0; i < 4; i++) {
            state[Nb*i+j] = res[i];
        }
    }
}

/*---------------------------------------------------*/

void MainWindow::decrypt(){
    //keyExpansion();

    QByteArray encodedText = QByteArray::fromHex(ui->textBeforeField->toPlainText().toLocal8Bit());
    QByteArray decodedText(encodedText);

    int index = 0;
    int amountOfBlocks = (encodedText.size() * 8) / 128;
    for(int block = 0; block < amountOfBlocks; block++){

        for(int i = 0; i < 16; i++){
            tempForState[i] = encodedText[i + index];
        }


        for (int i = 0; i < Nb; i++) {
            for (int j = 0; j < Nb; j++) {
                state[Nb*i + j] = tempForState[Nb*j + i];
            }
        }


        addRoundKey(Nr);

        for(int round = Nr - 1; round >= 1; round--){
            invShiftRows();
            invSubBytes();
            addRoundKey(round);
            invMixColumns();

        }

        invShiftRows();
        invSubBytes();
        addRoundKey(0);

            for (int i = 0; i < 4; i++) {
            for (int j = 0; j < Nb; j++) {
                decodedText[i+4*j+index] = state[Nb*i+j];
            }
        }
        index += 16;
    }

    ui->textAftedField->setPlainText(decodedText);
    encodedText.clear();
    decodedText.clear();

}

void MainWindow::invShiftRows(){
    uint8_t i, k, s, tmp;

    for (i = 1; i < 4; i++) {
        s = 0;
        while (s < i) {
            tmp = state[Nb*i+Nb-1];

            for (k = Nb-1; k > 0; k--) {
                state[Nb*i+k] = state[Nb*i+k-1];
            }

            state[Nb*i+0] = tmp;
            s++;
        }
    }
}

void MainWindow::invSubBytes(){
    uint8_t i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < Nb; j++) {
            state[Nb*i+j] = inv_s_box[state[Nb*i+j]];
        }
    }
}

void MainWindow::invMixColumns(){
    uint8_t a[] = {0x0e, 0x09, 0x0d, 0x0b}; // a(x) = {0e} + {09}x + {0d}x2 + {0b}x3
    uint8_t i, j, col[4], res[4];

    for (j = 0; j < Nb; j++) {
        for (i = 0; i < 4; i++) {
            col[i] = state[Nb*i+j];
        }

        coef_mult(a, col, res);

        for (i = 0; i < 4; i++) {
            state[Nb*i+j] = res[i];
        }
    }
}

/*---------------------------------------------------*/

void MainWindow::keyExpansion(){
    uint8_t tmp[4];
    uint8_t i;
    uint8_t len = Nb*(Nr+1);

    for (i = 0; i < Nk; i++) {
        MainWindow::expandedKey[4*i+0] = MainWindow::key[4*i+0];
        MainWindow::expandedKey[4*i+1] = MainWindow::key[4*i+1];
        MainWindow::expandedKey[4*i+2] = MainWindow::key[4*i+2];
        MainWindow::expandedKey[4*i+3] = MainWindow::key[4*i+3];
    }

    for (i = Nk; i < len; i++) {
        tmp[0] = MainWindow::expandedKey[4*(i-1)+0];
        tmp[1] = MainWindow::expandedKey[4*(i-1)+1];
        tmp[2] = MainWindow::expandedKey[4*(i-1)+2];
        tmp[3] = MainWindow::expandedKey[4*(i-1)+3];

        if (i%Nk == 0) {
            rot_word(tmp);
            sub_word(tmp);
            coef_add(tmp, Rcon(i/Nk), tmp);
        }
        else if (Nk > 6 && i%Nk == 4) {
            sub_word(tmp);
        }

        MainWindow::expandedKey[4*i+0] = MainWindow::expandedKey[4*(i-Nk)+0]^tmp[0];
        MainWindow::expandedKey[4*i+1] = MainWindow::expandedKey[4*(i-Nk)+1]^tmp[1];
        MainWindow::expandedKey[4*i+2] = MainWindow::expandedKey[4*(i-Nk)+2]^tmp[2];
        MainWindow::expandedKey[4*i+3] = MainWindow::expandedKey[4*(i-Nk)+3]^tmp[3];
    }
}

void MainWindow::rot_word(uint8_t *w) {

    uint8_t tmp;
    uint8_t i;

    tmp = w[0];

    for (i = 0; i < 3; i++) {
        w[i] = w[i+1];
    }

    w[3] = tmp;
}

void MainWindow::sub_word(uint8_t *w) {

    uint8_t i;

    for (i = 0; i < 4; i++) {
        w[i] = s_box[w[i]];
    }
}

void MainWindow::coef_add(uint8_t a[], uint8_t b[], uint8_t d[]) {

    d[0] = a[0]^b[0];
    d[1] = a[1]^b[1];
    d[2] = a[2]^b[2];
    d[3] = a[3]^b[3];
}

uint8_t * MainWindow::Rcon(uint8_t i) {

    if (i == 1) {
        MainWindow::rcon[0] = 0x01; // x^(1-1) = x^0 = 1
    } else if (i > 1) {
        MainWindow::rcon[0] = 0x02;
        i--;
        while (i > 1) {
            MainWindow::rcon[0] = gmult(MainWindow::rcon[0], 0x02);
            i--;
        }
    }

    return MainWindow::rcon;
}

uint8_t MainWindow::gmult(uint8_t a, uint8_t b) {
    uint8_t p = 0, i = 0, hbs = 0;
    for (i = 0; i < 8; i++) {
        if (b & 1) {
            p ^= a;
        }
        hbs = a & 0x80;
        a <<= 1;
        if (hbs) a ^= 0x1b; // 0000 0001 0001 1011
        b >>= 1;
    }
    return (uint8_t)p;
}

void MainWindow::coef_mult(uint8_t *a, uint8_t *b, uint8_t *d){
    d[0] = gmult(a[0],b[0])^gmult(a[3],b[1])^gmult(a[2],b[2])^gmult(a[1],b[3]);
    d[1] = gmult(a[1],b[0])^gmult(a[0],b[1])^gmult(a[3],b[2])^gmult(a[2],b[3]);
    d[2] = gmult(a[2],b[0])^gmult(a[1],b[1])^gmult(a[0],b[2])^gmult(a[3],b[3]);
    d[3] = gmult(a[3],b[0])^gmult(a[2],b[1])^gmult(a[1],b[2])^gmult(a[0],b[3]);
}

