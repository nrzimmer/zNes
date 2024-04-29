#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCoreApplication>
#include <QFile>
#include <QByteArray>
#include <iostream>
#include <QPainter>
#include <QTimer>

chr_bank *MainWindow::chrData = 0;
uint8_t MainWindow::prgData[0x8000] = {};
uint8_t MainWindow::ram[2048] = {};
mos6502 *MainWindow::cpu = 0;


void MainWindow::stepEmu()
{
    std::cout << std::hex << "PC: " << cpu->pc << "\tSP:" << (int)cpu->sp << "\tST:" << (int)cpu->status << "\tA:" << (int)cpu->A << "\tX:" << (int)cpu->X << "\tY:" << (int)cpu->Y << std::endl;
    cpu->Run(1);
}

void MainWindow::btnClicked()
{
    unsigned char NES_HEADER[4] = {0x4E, 0x45, 0x53, 0x1A};
    QFile fRom("/home/zimmer/src/zNes/smb.nes");
    //QFile fRom("D:\\Qt Projects\\zNes\\wreckingcrew.nes");
    QByteArray arrRom;

    uint32_t nPRG_ROM_PAGES;
    uint32_t nCHR_ROM_PAGES;
    MirroringType mtMirroring;
    bool bHasPRG_RAM;
    bool bHasTrainer;
    bool bIgnoreMirroring;
    bool bVsUnisystem;
    bool bPlaychoice;
    bool bNes2Header;
    uint32_t nMapper;

//    char* arrPRG_ROM;
    char* arrCHR_ROM;
//    char* arrTRAINER;

    if (!fRom.open(QIODevice::ReadOnly))
        return;

    arrRom = fRom.read(16);

    if (*(uint32_t*)arrRom.data() == *(uint32_t*)&NES_HEADER)
    {
        nPRG_ROM_PAGES = (unsigned char)arrRom.at(4);
        nCHR_ROM_PAGES = (unsigned char)arrRom.at(5);
        mtMirroring = ((arrRom.at(6) && 0x1) == 0 ? HORIZONTAL : VERTICAL);
        bHasPRG_RAM = arrRom.at(6) && 0x2;
        bHasTrainer = arrRom.at(6) && 0x4;
        bIgnoreMirroring = arrRom.at(6) && 0x8;
        nMapper = (arrRom.at(6) && 0xF0) >> 4;

        bVsUnisystem = arrRom.at(7);
        bPlaychoice = arrRom.at(7) && 0x2;
        bNes2Header = arrRom.at(7) && 0x8;
        nMapper &= (arrRom.at(7) && 0xF0);

        std::cout << "NES" << std::endl;
        std::cout << "PRG ROM: " << nPRG_ROM_PAGES * 16 << "KB" << std::endl;
        std::cout << "CHR ROM: " << nCHR_ROM_PAGES * 8 << "KB" << std::endl;
        std::cout << "Mirroring: " << (mtMirroring == HORIZONTAL ? "Horizontal" : "Vertical") << std::endl;
        std::cout << "PRG RAM: " << bHasPRG_RAM << std::endl;
        std::cout << "Has Trainer: " << bHasTrainer << std::endl;
        std::cout << "Ignore Mirroring: " << bIgnoreMirroring << std::endl;
        std::cout << "Vs Unisystem: " << bVsUnisystem << std::endl;
        std::cout << "Playchoice: " << bPlaychoice << std::endl;
        std::cout << "NES 2.0 Header: " << bNes2Header << std::endl;
        std::cout << "Mapper: " << nMapper << std::endl;

        //arrPRG_ROM = new char[nPRG_ROM_PAGES * 16 * 1024];
        arrCHR_ROM = new char[nCHR_ROM_PAGES *  8 * 1024];
/*        if (bHasTrainer)
        {
            arrTRAINER = new char[512];
            fRom.read(arrTRAINER, 512);
        } */
        //need check for size
        fRom.read((char*)&prgData, nPRG_ROM_PAGES * 16 * 1024);

        int scale = 3;

        QPixmap grade(33*2+32*8*scale,17*2+16*8*scale);
        grade.fill(Qt::white);
        QPainter pntGrade(&grade);
        pntGrade.setPen(Qt::black);
        int lX = 0;
        for (int i=0; lX < grade.width(); i++)
        {
            pntGrade.drawLine(QPoint(lX,0), QPoint(lX,grade.height()));
            lX += 1;
            pntGrade.drawLine(QPoint(lX,0), QPoint(lX,grade.height()));
            lX += 25;
        }
        int lY = 0;
        for (int i=0; lY < grade.height(); i++)
        {
            pntGrade.drawLine(QPoint(0,lY), QPoint(grade.width(),lY));
            lY += 1;
            pntGrade.drawLine(QPoint(0,lY), QPoint(grade.width(),lY));
            lY += 25;
        }

        for (unsigned int nCurrentCHR=0;nCurrentCHR < nCHR_ROM_PAGES; nCurrentCHR++)
        {
            fRom.read(arrCHR_ROM, 8 * 1024);

            chrData = new chr_bank[nCHR_ROM_PAGES];
            //Tile decode
            int x = 2;
            int xbase = 2;
            int y = 2;
            for (int nCurrentTile=0; nCurrentTile < 512; nCurrentTile++)
            {
                for (int i=0; i<8; i++)
                {
                    int chrLo, chrHi;
                    chrLo = (unsigned char)arrCHR_ROM[(nCurrentTile*16)+i];
                    chrHi = (unsigned char)arrCHR_ROM[(nCurrentTile*16)+i+8];
                    chrData[nCurrentCHR].tiles[nCurrentTile].data[i][7] = (((chrLo & 0x01) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x01) > 0) ? (uint8_t)2 : 0);
                    chrData[nCurrentCHR].tiles[nCurrentTile].data[i][6] = (((chrLo & 0x02) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x02) > 0) ? (uint8_t)2 : 0);
                    chrData[nCurrentCHR].tiles[nCurrentTile].data[i][5] = (((chrLo & 0x04) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x04) > 0) ? (uint8_t)2 : 0);
                    chrData[nCurrentCHR].tiles[nCurrentTile].data[i][4] = (((chrLo & 0x08) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x08) > 0) ? (uint8_t)2 : 0);
                    chrData[nCurrentCHR].tiles[nCurrentTile].data[i][3] = (((chrLo & 0x10) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x10) > 0) ? (uint8_t)2 : 0);
                    chrData[nCurrentCHR].tiles[nCurrentTile].data[i][2] = (((chrLo & 0x20) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x20) > 0) ? (uint8_t)2 : 0);
                    chrData[nCurrentCHR].tiles[nCurrentTile].data[i][1] = (((chrLo & 0x40) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x40) > 0) ? (uint8_t)2 : 0);
                    chrData[nCurrentCHR].tiles[nCurrentTile].data[i][0] = (((chrLo & 0x80) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x80) > 0) ? (uint8_t)2 : 0);
                }
                chrData[nCurrentCHR].tiles[nCurrentTile].pic = new QPixmap(8,8);
                chrData[nCurrentCHR].tiles[nCurrentTile].pic->fill(Qt::transparent);
                QPainter painter(chrData[nCurrentCHR].tiles[nCurrentTile].pic);
                drawTile(QPoint(0,0), &painter, nCurrentCHR, nCurrentTile, 1);
                pntGrade.drawPixmap(QRect(x,y,scale*8,scale*8), chrData[nCurrentCHR].tiles[nCurrentTile].pic->scaledToWidth(scale*8, Qt::FastTransformation));
                x += scale*8+2;
                if (xbase == 2)
                {
                    if (x >= (int)grade.width() / 2)
                    {
                        x = xbase;
                        y += scale * 8 + 2;
                    }
                }
                else
                {
                    if (x >= grade.width())
                    {
                        x = xbase;
                        y += scale * 8 + 2;
                    }
                }
                if (y >= grade.height())
                {
                    xbase = 16*scale*8+17*2;
                    x = xbase;
                    y = 2;
                }
            }
            ui->label->setPixmap(grade);
        }
    }
    index = 0;
    QTimer *timer = new QTimer;
    timer->start(90);
    connect(timer,SIGNAL(timeout()),this,SLOT(incIndex()));
    ui->centralWidget->update();
    cpu = new mos6502(&MemRead, &MemWrite);
    cpu->Reset();
}

void MainWindow::incIndex()
{
    index++;
    if (index > 2) index = 0;
    ui->centralWidget->update();
}

void MainWindow::paintEvent(QPaintEvent *)
{
    if (chrData == nullptr) return;

    //create a QPainter and pass a pointer to the device.
    //A paint device can be a QWidget, a QPixmap or a QImage
    QPixmap scene(16,32);
    scene.fill(Qt::transparent);
    QPainter pntScene(&scene);
    //    painter.setRenderHint(QPainter::Antialiasing,true);

    uint8_t scale =1;

    int x, y;
    y = -8*scale;
    for (int i=index*8; i<8+index*8;i++)
    {
        if (i % 2 == 0)
        {
            x = 0;
            y += 8*scale;
        }
        else
        {
            x = scale*8;
        }
        pntScene.drawPixmap(QRect(x,y,scale*8,scale*8), chrData[0].tiles[i].pic->scaledToWidth(scale*8, Qt::FastTransformation));
        //drawTile(QPoint(x,y), &painter, 0, i, scale);
    }
    QPainter painter;
    scale = 4;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    //painter.drawPixmap(QRect(10,10,scale*20,scale*36), scene.scaledToWidth(scale*20, Qt::SmoothTransformation));
    painter.drawPixmap(QRect(10,10,scale*scene.width(),scale*scene.height()), scene.scaledToWidth(scale*scene.width(), Qt::FastTransformation));
    painter.end();
}

void MainWindow::drawTile(QPoint ptPos, QPainter *painter, uint16_t nBank, uint16_t nTile, uint8_t nScale)
{
    for (int y=0; y<8; y++)
    {
        for (int x=0; x<8; x++)
        {
            switch(chrData[nBank].tiles[nTile].data[y][x])
            {
            case 0:
                painter->setPen(Qt::transparent);
                break;
            case 1:
                painter->setPen(Qt::red);
                break;
            case 2:
                painter->setPen(QColor(255,112,0));
                break;
            case 3:
                painter->setPen(QColor(163,97,40));
                break;
            }
            for (int nScaleX=0;nScaleX<nScale;nScaleX++)
            {
                for (int nScaleY=0;nScaleY<nScale;nScaleY++)
                {
                    painter->drawPoint(ptPos.x()+nScaleX,ptPos.y()+nScaleY);
                }
            }
            ptPos.rx() += nScale;
        }
        ptPos.rx() -= 8*nScale;
        ptPos.ry() += nScale;
    }
}

void MainWindow::MemWrite(uint16_t addr, uint8_t value)
{
    std::cout << "MemWrite: " << std::hex <<  addr << " = " << (int)value << std::endl;
    if (addr < 0x2000) //RAM
    {
        ram[addr & 0x07FF] = value;
    }
    else if (addr < 0x4000) //PPU
    {

    }
    else if (addr < 0x4020) //APU AND IO
    {

    }
    else if (addr < 0x8000) //ROM
    {
        //Nope
    }
    else //ROM
    {
        //Nope
    }

}

uint8_t MainWindow::MemRead(uint16_t addr)
{
    uint8_t resval;
    if (addr < 0x2000) //RAM
    {
        resval = ram[addr & 0x07FF];
    }
    else if (addr < 0x4000) //PPU
    {
        resval = 0;
    }
    else if (addr < 0x4020) //APU AND IO
    {
        resval = 0;
    }
    else if (addr < 0x8000) //ROM
    {
        resval = 0;
    }
    else //ROM
    {
        resval = prgData[addr & 0x7FFF];
    }
    std::cout << "MemRead: " << std::hex << addr << " = " << (int)resval << std::endl;
    return resval;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    chrData = nullptr;
    //btnClicked();
    ui->setupUi(this);
    connect(ui->pushButton, SIGNAL (released()), this, SLOT (btnClicked()));
    connect(ui->pushButton_2, SIGNAL (released()), this, SLOT (stepEmu()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
