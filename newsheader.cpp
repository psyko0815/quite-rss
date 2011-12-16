#include "newsheader.h"

NewsHeader::NewsHeader(Qt::Orientation orientation, QWidget * parent) :
    QHeaderView(orientation, parent)
{
  setObjectName("newsHeader");
  setContextMenuPolicy(Qt::CustomContextMenu);
  setMovable(true);
  setDefaultAlignment(Qt::AlignLeft);
  setMinimumSectionSize(25);
  setStretchLastSection(true);

  viewMenu_ = new QMenu(this);

  buttonColumnView = new QPushButton();
  buttonColumnView->setIcon(QIcon(":/images/images/triangleT.png"));
  buttonColumnView->setObjectName("buttonColumnView");
  buttonColumnView->setMaximumWidth(30);
  connect(buttonColumnView, SIGNAL(clicked()), this, SLOT(slotButtonColumnView()));

  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->setMargin(0);
  buttonLayout->addWidget(buttonColumnView, 0, Qt::AlignRight|Qt::AlignVCenter);
  setLayout(buttonLayout);

  this->installEventFilter(this);
}

void NewsHeader::init()
{
  QActionGroup *pActGroup_ = new QActionGroup(viewMenu_);
  pActGroup_->setExclusive(false);
  connect(pActGroup_, SIGNAL(triggered(QAction*)), this, SLOT(columnVisibled(QAction*)));
  for (int i = 0; i < model_->columnCount(); i++) {
    QAction *action = pActGroup_->addAction(
          model_->headerData(logicalIndex(i),
          Qt::Horizontal, Qt::EditRole).toString());
    action->setData(logicalIndex(i));
    action->setCheckable(true);
    action->setChecked(!isSectionHidden(logicalIndex(i)));
    viewMenu_->addAction(action);
  }
}

void NewsHeader::overload()
{
  for (int i = 0; i < model_->columnCount(); i++) {
    model_->setHeaderData(i, Qt::Horizontal,
                          model_->headerData(i, Qt::Horizontal, Qt::DisplayRole),
                          Qt::EditRole);
  }
  model_->setHeaderData(model_->fieldIndex("title"), Qt::Horizontal, tr("Title"), Qt::DisplayRole);
  model_->setHeaderData(model_->fieldIndex("published"), Qt::Horizontal, tr("Date"), Qt::DisplayRole);
  model_->setHeaderData(model_->fieldIndex("received"), Qt::Horizontal, tr("Received"), Qt::DisplayRole);
  model_->setHeaderData(model_->fieldIndex("read"), Qt::Horizontal, "", Qt::DisplayRole);
  model_->setHeaderData(model_->fieldIndex("read"), Qt::Horizontal, QIcon(":/images/markRead"), Qt::DecorationRole);

  for (int i = 0; i < model_->columnCount(); i++) {
//    qDebug() << model_->headerData(i, Qt::Horizontal, Qt::EditRole).toString();
  }


}

bool NewsHeader::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::Resize) {
    QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event);
    bool minSize = false;
    int oldWidth = resizeEvent->oldSize().width();
    int newWidth = resizeEvent->size().width();
    if (oldWidth > 0) {
      int size = 0;
      int widthCol[count()];
      memset(widthCol, 0, sizeof(widthCol));
      static int idxColSize = count()-1;
      int tWidth = 0;
      for (int i = 0; i < count(); i++) tWidth += sectionSize(i);
      if (tWidth > newWidth) {
        minSize = true;
        size = tWidth - newWidth;
      } else {
        size = newWidth - tWidth;
      }
      int countCol = 0;
      bool sizeOne = false;
      while (size) {
        int lIdx = logicalIndex(idxColSize);
        if (!isSectionHidden(lIdx)) {
          if (((sectionSize(lIdx) >= 40) && !minSize) ||
              ((sectionSize(lIdx) - widthCol[idxColSize] > 40) && minSize)) {
            widthCol[idxColSize]++;
            size--;
            sizeOne = true;
          }
        }
        if (idxColSize == 0) idxColSize = count()-1;
        else idxColSize--;

        if (++countCol == count()) {
          if (!sizeOne) break;
          sizeOne = false;
          countCol = 0;
        }
      }

      for (int i = count()-1; i >= 0; i--) {
        int lIdx = logicalIndex(i);
        if (!isSectionHidden(lIdx) && (sectionSize(lIdx) >= 40)) {
          if (!minSize) {
            resizeSection(lIdx, sectionSize(lIdx) + widthCol[i]);
          } else {
            resizeSection(lIdx, sectionSize(lIdx) - widthCol[i]);
          }
        }
      }
      event->ignore();
      return true;
    } else return false;
  } else {
    return false;
  }
}

/*virtual*/ void NewsHeader::mousePressEvent(QMouseEvent *event)
{
  QPoint nPos = event->pos();
  nPos.setX(nPos.x() + 5);
  idxCol = visualIndex(logicalIndexAt(nPos));
  posX = event->pos().x();
  nPos = event->pos();
  nPos.setX(nPos.x() - 5);
  QHeaderView::mousePressEvent(event);
}

/*virtual*/ void NewsHeader::mouseMoveEvent(QMouseEvent *event)
{
  bool sizeMin = false;
  if (event->buttons() & Qt::LeftButton) {
    int oldWidth = width();
    int newWidth = 0;
    int stopColFix = 0;
    for (int i = count()-1; i >= 0; i--) {
      if (!isSectionHidden(i)) {
        stopColFix = visualIndex(i);
        break;
      }
    }
    for (int i = 0; i < count(); i++) newWidth += sectionSize(i);
    if (posX > event->pos().x()) sizeMin =  true;
    if (!sizeMin) {
      if (event->pos().x() < oldWidth) {
        for (int i = count()-1; i >= 0; i--) {
          int lIdx = logicalIndex(i);
          if (!isSectionHidden(lIdx)) {
            int sectionWidth = sectionSize(lIdx) + oldWidth - newWidth;
            if (sectionWidth > 40) {
              if (i >= idxCol) {
                resizeSection(lIdx, sectionWidth);
                sizeMin = true;
                break;
              }
            }
          }
        }
      }
      int tWidth = 0;
      for (int i = idxCol; i < count(); i++) {
        if (!isSectionHidden(logicalIndex(i))) {
          tWidth += 40;
        }
      }
      if (event->pos().x()+tWidth > oldWidth) {
        sizeMin = false;
      }
    } else {
      int sectionWidth = sectionSize(logicalIndex(stopColFix)) + oldWidth - newWidth;
      if ((sectionWidth > 40)) {
        resizeSection(logicalIndex(stopColFix), sectionWidth);
      }
    }
    if (!sizeMin) {
      if (posX > event->pos().x()) posX = event->pos().x();
      event->ignore();
      return;
    }
  }
  if (posX > event->pos().x()) posX = event->pos().x();
  QHeaderView::mouseMoveEvent(event);
}

void NewsHeader::slotButtonColumnView()
{
  viewMenu_->setFocus();
  viewMenu_->show();
  QPoint pPoint;
  pPoint.setX(mapToGlobal(QPoint(0,0)).x() + width() - viewMenu_->width());
  pPoint.setY(mapToGlobal(QPoint(0,0)).y() + height());
  viewMenu_->popup(pPoint);
}

void NewsHeader::columnVisibled(QAction *action)
{
  int idx = action->data().toInt();
  setSectionHidden(idx, !isSectionHidden(idx));
}
