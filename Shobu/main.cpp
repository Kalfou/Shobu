#include "shobuview.h"

#include <QApplication>

// starts the program
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ShobuView main_window;
    main_window.show();
    return a.exec();
}
