#ifndef TAGSLINEWIDGET_H
#define TAGSLINEWIDGET_H

#include <QWidget>
#include <QApplication>
#include <QStyleOptionFrame>

#include "tagsmemento.h"

const int vertical_margin = 3;
const int bottommargin = 1;
const int topmargin = 1;

const int horizontal_margin = 3;
const int leftmargin = 1;
const int rightmargin = 1;

class TagsLineEditWidget : public QWidget {
    Q_OBJECT

public:
    explicit TagsLineEditWidget(QWidget* parent);
    ~TagsLineEditWidget();

    // QWidget
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    /// Set completions
    //void completion(std::vector<QString> const& completions);

    void SetTags(const QVector<QString> &SetTags);
    QVector<QString> GetTags() const;

Q_SIGNALS:
    void ToTagsEdited();

protected:
    // QWidget
    void paintEvent(QPaintEvent* event) override;
    void timerEvent(QTimerEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void RepaintWidget();
    TagsPresenter *m_tagsPresenter;

    // QWidget interface
protected:
    void wheelEvent(QWheelEvent *event) override;
};
#endif // TAGSLINEWIDGET_H
