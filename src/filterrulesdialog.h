#ifndef FILTERRULES_H
#define FILTERRULES_H

#include "dialog.h"
#include "lineedit.h"

class ItemCondition : public QWidget
{
  Q_OBJECT
private:
  QToolButton *deleteButton;

public:
  ItemCondition(QWidget * parent = 0) : QWidget(parent)
  {
    comboBox1 = new QComboBox(this);
    comboBox2 = new QComboBox(this);
    comboBox3 = new QComboBox(this);
    lineEdit = new LineEdit(this);

    QStringList itemList;
    itemList << tr("Title")  << tr("Description")
             << tr("Author") << tr("Category") << tr("Status")
                /*<< tr("Published") << tr("Received")*/;
    comboBox1->addItems(itemList);

    itemList.clear();
    itemList << tr("New") << tr("Read") << tr("Starred");
    comboBox3->addItems(itemList);

    currentIndexChanged(tr("Title"));

    addButton = new QToolButton(this);
    addButton->setIcon(QIcon(":/images/addT"));
    addButton->setToolTip(tr("Add Condition"));
    addButton->setAutoRaise(true);
    deleteButton = new QToolButton(this);
    deleteButton->setIcon(QIcon(":/images/deleteT"));
    deleteButton->setToolTip(tr("Delete Condition"));
    deleteButton->setAutoRaise(true);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setAlignment(Qt::AlignCenter);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(5);
    buttonsLayout->addWidget(comboBox1);
    buttonsLayout->addWidget(comboBox2);
    buttonsLayout->addWidget(comboBox3, 1);
    buttonsLayout->addWidget(lineEdit, 1);
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(deleteButton);

    setLayout(buttonsLayout);

    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteFilterRules()));
    connect(comboBox1, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(currentIndexChanged(QString)));
  }

  QComboBox *comboBox1;
  QComboBox *comboBox2;
  QComboBox *comboBox3;
  LineEdit *lineEdit;

  QToolButton *addButton;

private slots:
  void deleteFilterRules()
  {
    emit signalDeleteCondition(this);
  }

  void currentIndexChanged(QString str)
  {
    QStringList itemList;

    comboBox2->clear();
    comboBox3->setVisible(false);
    lineEdit->setVisible(true);
    if (str == tr("Title")) {
      itemList << tr("contains") << tr("doesn't contains")
               << tr("is") << tr("isn't")
               << tr("begins with") << tr("ends with");
      comboBox2->addItems(itemList);
    } else if (str == tr("Description")) {
      itemList << tr("contains") << tr("doesn't contains");
      comboBox2->addItems(itemList);
    } else if (str == tr("Author")) {
      itemList << tr("contains") << tr("doesn't contains")
               << tr("is") << tr("isn't");
      comboBox2->addItems(itemList);
    } else if (str == tr("Category")) {
      itemList << tr("is") << tr("isn't")
               << tr("begins with") << tr("ends with");
      comboBox2->addItems(itemList);
    } else if (str == tr("Status")) {
      itemList << tr("is") << tr("isn't");
      comboBox2->addItems(itemList);
      comboBox3->setVisible(true);
      lineEdit->setVisible(false);
    } /*else if (str == "Published") {
      itemList << tr("is") << tr("isn't")
               << tr("is before") << tr("is after");
      comboBox2->addItems(itemList);
    } else if (str == "Received") {
      itemList << tr("is") << tr("isn't")
               << tr("is before") << tr("is after");
      comboBox2->addItems(itemList);
    }*/
  }

signals:
  void signalDeleteCondition(ItemCondition *item);
};

class ItemAction : public QWidget
{
  Q_OBJECT
private:
  QToolButton *deleteButton;

public:
  ItemAction(QWidget * parent = 0) : QWidget(parent)
  {
    QStringList itemList;
    comboBox1 = new QComboBox(this);
    itemList /*<< tr("Move News to")  << tr("Copy News to")*/
             << tr("Mark News as Read") << tr("Add Star")
             << tr("Delete") << tr("Add Label");
    comboBox1->addItems(itemList);

    comboBox2 = new QComboBox(this);
    comboBox2->setVisible(false);

    addButton = new QToolButton(this);
    addButton->setIcon(QIcon(":/images/addT"));
    addButton->setToolTip(tr("Add Action"));
    addButton->setAutoRaise(true);
    deleteButton = new QToolButton(this);
    deleteButton->setIcon(QIcon(":/images/deleteT"));
    deleteButton->setToolTip(tr("Delete Action"));
    deleteButton->setAutoRaise(true);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setAlignment(Qt::AlignCenter);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(5);
    buttonsLayout->addWidget(comboBox1);
    buttonsLayout->addWidget(comboBox2);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(deleteButton);

    setLayout(buttonsLayout);

    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteFilterAction()));
    connect(comboBox1, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChanged(int)));
  }

  QComboBox *comboBox1;
  QComboBox *comboBox2;
  QToolButton *addButton;

private slots:
  void deleteFilterAction()
  {
    emit signalDeleteAction(this);
  }

  void currentIndexChanged(int index)
  {
    if (index == 3) comboBox2->setVisible(true);
    else comboBox2->setVisible(false);
  }

signals:
  void signalDeleteAction(ItemAction *item);
};

class FilterRulesDialog : public Dialog
{
  Q_OBJECT
private:
  void setData();

  QSettings *settings_;
  LineEdit *filterName;
  QTreeWidget *feedsTree;

  QComboBox *matchComboBox_;

  QScrollArea *conditionScrollArea;
  QVBoxLayout *conditionLayout;
  QWidget *conditionWidget;

  QScrollArea *actionsScrollArea;
  QVBoxLayout *actionsLayout;
  QWidget *actionsWidget;

  QLabel *textWarning;
  QWidget *warningWidget_;

public:
  explicit FilterRulesDialog(QWidget *parent, int filterId, int feedId = -1);

  int filterId_;
  bool itemNotChecked_;

signals:

public slots:

private slots:
  void feedItemChanged(QTreeWidgetItem *item, int column);
  void closeDialog();
  void acceptDialog();

  void filterNameChanged(const QString &text);

  ItemCondition *addCondition();
  void deleteCondition(ItemCondition *item);

  ItemAction *addAction();
  void deleteAction(ItemAction *item);

  void selectMatch(int index);

};

#endif // FILTERRULES_H
