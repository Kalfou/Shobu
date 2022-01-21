#ifndef SHOBUEXCEPTION_H
#define SHOBUEXCEPTION_H

#include <QException>

class ShobuException : public QException
{
public:
    void raise() const override { throw *this; }
    ShobuException *clone() const override { return new ShobuException(*this); }
    void setMessage(QString msg) {message = msg;}
    QString getMessage() const {return message;}

private:
    QString message;
};

#endif // SHOBUEXCEPTION_H
