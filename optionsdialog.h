#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QtGui>
#include <QNetworkProxy>

class OptionsDialog : public QDialog
{
  Q_OBJECT
public:
  explicit OptionsDialog(QWidget *parent = 0);
  QNetworkProxy proxy();
  void setProxy(const QNetworkProxy proxy);

private slots:
  void slotCategoriesItemCLicked(QTreeWidgetItem* item, int column);
  void manualProxyToggle(bool checked);
  void updateProxy();
  void applyProxy();
  void acceptSlot();

signals:

private:
  QLabel *contentLabel_;
  QStackedWidget *contentStack_;

  //stack widgets
  QWidget *networkConnectionsWidget_;

  QDialogButtonBox *buttonBox_;

  // network connection
  QRadioButton *systemProxyButton_;
  QRadioButton *directConnectionButton_;
  QRadioButton *manualProxyButton_;
  QWidget *manualWidget_;
  QLineEdit *editHost_;
  QLineEdit *editPort_;
  QLineEdit *editUser_;
  QLineEdit *editPassword_;

  // internal variables for options
  QNetworkProxy networkProxy_;
};

#endif // OPTIONSDIALOG_H
