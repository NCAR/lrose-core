
#ifndef DIALOG_H
#define DIALOG_H

#include <QWidget>
class QCheckBox;
class QLabel;
class QLineEdit;
class QErrorMessage;
class QPushButton;
class DialogOptionsWidget;

class Dialog : public QWidget
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

private slots:
    void setInteger();
    void setDouble();
    void setItem();
    void setText();
    void setMultiLineText();
    void setCenterPoint();
    void setGridColor();
    void setBoundaryColor();
    void setExceededColor();
    void setMissingColor();
    void setAnnotationColor();
    void setBackgroundColor();
    void setEmphasisColor();
    void saveColorScale();
    void setFont();
    void setExistingDirectory();
    void setOpenFileName();
    void setOpenFileNames();
    void setSaveFileName();
    void criticalMessage();
    void informationMessage();
    void questionMessage();
    void warningMessage();
    void errorMessage();

private:

    double newCenterPoint;

    QLabel *integerLabel;
    QLabel *doubleLabel;
    QLabel *itemLabel;
    QLabel *textLabel;
    QLabel *multiLineTextLabel;

    QLabel *centerColorLabel;
    QLineEdit *centerColorLineEdit;

    QLabel *gridColorLabel;
    QPushButton *gridColorButton;
    QLabel *boundaryColorLabel;
    QPushButton *boundaryColorButton;
    QLabel *exceededColorLabel;
    QPushButton *exceededColorButton;
    QLabel *missingColorLabel;
    QPushButton *missingColorButton;
    QLabel *annotationColorLabel;
    QPushButton *annotationColorButton;
    QLabel *backgroundColorLabel;
    QPushButton *backgroundColorButton;
    QLabel *emphasisColorLabel;
    QPushButton *emphasisColorButton;
    QLabel *colorSample;
    
    QLabel *fontLabel;
    QLabel *directoryLabel;
    QLabel *openFileNameLabel;
    QLabel *openFileNamesLabel;
    QLabel *saveFileNameLabel;
    QLabel *criticalLabel;
    QLabel *informationLabel;
    QLabel *questionLabel;
    QLabel *warningLabel;
    QLabel *errorLabel;
    QErrorMessage *errorMessageDialog;
    DialogOptionsWidget *fileDialogOptionsWidget;
    DialogOptionsWidget *colorDialogOptionsWidget;
    DialogOptionsWidget *fontDialogOptionsWidget;
    QString openFilesPath;
};

#endif
