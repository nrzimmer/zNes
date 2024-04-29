#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include "mos6502.h"

enum MirroringType
{
    VERTICAL,
    HORIZONTAL
};

typedef struct {
    int data[8][8];
    QPixmap *pic;
} s_tile;

typedef struct {
    s_tile tiles[512];
} chr_bank;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void stepEmu();
    void btnClicked();
    void incIndex();
protected:
    void paintEvent(QPaintEvent *);
    void drawTile(QPoint ptPos, QPainter *painter, uint16_t nBank, uint16_t nTile, uint8_t nScale);
    uint8_t index;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static void MemWrite(uint16_t addr, uint8_t value);
    static uint8_t MemRead(uint16_t addr);
    static chr_bank *chrData;
    static uint8_t prgData[0x8000];
    static uint8_t ram[2048];
    static mos6502 *cpu;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
