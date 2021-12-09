#ifndef TAGSMEMENTO_H
#define TAGSMEMENTO_H
#include <QVector>
#include <QPen>
#include <QtMath>
#include <QPainterPath>
#include <QPainter>
#include <QStyleHints>
#include <QGuiApplication>
#include <QTextLayout>
#include <QStyleOptionFrame>
#include <QtGui/private/qinputcontrol_p.h>

#include "tag.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
#define FONT_METRICS_WIDTH(fmt, ...) fmt.width(__VA_ARGS__)
#else
#define FONT_METRICS_WIDTH(fmt, ...) fmt.horizontalAdvance(__VA_ARGS__)
#endif

const int top_text_margin = 1;
const int bottom_text_margin = 1;
const int left_text_margin = 1;
const int right_text_margin = 1;

const int tag_horisontal_spacing = 3;
const int tag_inner_left_padding = 3;
const int tag_inner_right_padding = 4;
const int tag_inner_top_padding =3;
const int tag_inner_bottom_padding = 3;
const int tag_cross_width = 4;
const int tag_cross_spacing = 2;

class TagsPresenter : public QObject
{
    Q_OBJECT
public:
    TagsPresenter(QWidget *view);
    ~TagsPresenter();

    void InitStyleOptionFrame(QStyleOptionFrame* option) const;
    inline QRect GetCrossButtonRect(QRect const& r) const;
    bool IsPointInCrossRectArea(int tagIndex, QPoint const& point) const;
    void DrawTags(QPainter& p, int startIndex, int lastIndex, int row, int column) const;
    QRect GetInputWidgetRect() const;
    void CalculateRects();

    void SetCursorVisible(bool visible);

    void updateCursorBlinking();
    void UpdateDisplayText();


    QString const& GetCurrentEdittedTagText() const;


    QString& InsertCharectersInEdittedTagText(int startPos, const QString& string);
    QRect const& GetCurrentEdittedTagRect() const;
    void EditNewTag();
    QVector<QTextLayout::FormatRange> EditetTextFormating() const;
    bool hasSelection() const noexcept;
    void removeSelection();
    void removeBackwardOne();
    void SelectAll();
    void MoveCursor(int posX, int PosY, bool marked);
    void CalculateVecticalScroll(QRect const& editedTagRect);
    void EditPreviousTag();
    void EditNextTag();
    void SetTagEditableAtIndex(int i);
     int GetAllTagsHeight() const;
private:
    void CalculateTagsRects(QPoint& leftTopPoint, const QRect &widgetSizes, QVector<Tag> &Tags, int beginTagIndex, int lastTagIndex);
    void CalculateTagOnEdit(QPoint& leftTopPoint, const QRect &widgetSizes);
    bool cursorVisible() const;
    void setEditingIndex(int index);
    void SetCurrentText(QString const& text);
    QString& SetTextInEdittedTagText(const QString &text);
    QString& RemoveCharectersInEdittedTagText(int startPos, int count);
    QRect& SetCurrentEdittedTagRect(const QRect &rect);
    void DeselectAll();

public:
    QWidget* m_guiWidget;
    QVector<Tag> tags;
    int currentEditIndex;
    int m_cursorPosition;
    int m_cursorBlinkTimerId;
    bool m_cursorBlinkStatus;
    QTextLayout *m_textLayout;
    int select_start;
    int select_size;
    QInputControl m_inputControl;
    int m_vecticalScrollValue;
};

#endif // TAGSMEMENTO_H
