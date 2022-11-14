#include "mainwindow.h"

#include <QApplication>
#include <Windows.h>

int main(int argc, char *argv[])
{
    // disable QuickEdit mode in Console
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prev_mode;
    GetConsoleMode(hInput, &prev_mode);
    SetConsoleMode(hInput, prev_mode & ~ENABLE_QUICK_EDIT_MODE);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
