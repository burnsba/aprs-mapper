#ifndef BEEPVALIDATOR_H
#define BEEPVALIDATOR_H

#include <QIntValidator>

class beepvalidator : public QIntValidator
{
    Q_OBJECT
public:
    beepvalidator(int min, int max, QObject* parent);
    QValidator::State validate(QString& input, int&) const;

private:
    int b;
    int t;

signals:

public slots:

};

#endif // BEEPVALIDATOR_H
