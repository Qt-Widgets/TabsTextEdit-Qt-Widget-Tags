#include "tagsmemento.h"


TagsPresenter::TagsPresenter(QWidget *view)
    : QObject(view)
    , m_guiWidget(view)
    , currentEditIndex(0)
    , m_cursorPosition(0)
    , m_cursorBlinkTimerId(0)
    , m_cursorBlinkStatus(true)
    , m_textLayout(new QTextLayout())
    , select_start(0)
    , select_size(0)
    , m_inputControl(QInputControl::TextEdit)
    , hscroll(0)
{
    tags.append(Tag());
}

TagsPresenter::~TagsPresenter()
{

}

void TagsPresenter::InitStyleOptionFrame(QStyleOptionFrame *option) const
{
    Q_ASSERT(option);
    option->initFrom(m_guiWidget);
    option->rect = m_guiWidget->contentsRect();
    option->lineWidth = m_guiWidget->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, option, m_guiWidget);
    option->midLineWidth = 0;
    option->state |= QStyle::State_Sunken;
    option->features = QStyleOptionFrame::None;
}

QRect TagsPresenter::GetCrossButtonRect(const QRect &r) const
{
    QRect crossRect(QPoint(0, 0), QSize(tag_cross_width, tag_cross_width));
    crossRect.moveCenter(QPoint(r.right() - tag_cross_width, r.center().y()));
    return crossRect;
}

bool TagsPresenter::IsPointInCrossArea(int tag_index, const QPoint &point) const
{
    QRect CrossButtonRect(GetCrossButtonRect(tags.at(tag_index).rect));
    CrossButtonRect.adjust(-2, 0, 0, 0);
    CrossButtonRect.translate(-hscroll, 0);
    if(CrossButtonRect.contains(point))
    {
        if(!cursorVisible() || tag_index!=currentEditIndex)
        {
            return true;
        }
    }
    return false;
}

void TagsPresenter::DrawTags(QPainter &p, int startIndex, int lastIndex, int row, int column) const
{
    Q_ASSERT(startIndex>=0 && startIndex<=lastIndex);
    for (int i=startIndex; i< lastIndex; ++i)
    {
        // Рисуем прямоугольник тега
        QRect const& tagRect = tags.at(i).rect.translated(-hscroll, 0);
        const QColor blueColor(0, 96, 100, 150);
        QPainterPath path;
        path.addRoundedRect(tagRect, 4, 4);
        p.fillPath(path, blueColor);

        // Рисуем текст
        QPoint const textPositionTopLeft = tagRect.topLeft() +
                QPoint(tag_inner_left_padding,
                       m_guiWidget->fontMetrics().ascent() +
                       ((tagRect.height()/(row+1) - m_guiWidget->fontMetrics().height()) / 2*(row+1)));
        p.setPen(Qt::white);
        p.drawText(textPositionTopLeft, tags.at(i).text);

        // Высчитаваем крестик закрытия
        QRect crossRect = GetCrossButtonRect(tagRect);

        // Рисуем крестик закрытия
        QPen pen = p.pen();
        pen.setWidth(2);

        p.save();
        p.setPen(pen);
        p.setRenderHint(QPainter::Antialiasing);
        p.drawLine(QLineF(crossRect.topLeft(), crossRect.bottomRight()));
        p.drawLine(QLineF(crossRect.bottomLeft(), crossRect.topRight()));
        p.restore();
    }
}

QRect TagsPresenter::GetInputWidgetRect() const
{
    QStyleOptionFrame panel;
    InitStyleOptionFrame(&panel);
    QRect r = m_guiWidget->style()->subElementRect(QStyle::SE_LineEditContents, &panel, m_guiWidget);
    r.adjust(left_text_margin, top_text_margin, -right_text_margin, -bottom_text_margin);
    return r;
}

void TagsPresenter::CalculateRects()
{
    QRect widgetSizes = GetInputWidgetRect();
    QPoint leftTopPoint = widgetSizes.topLeft();

    CalculateTagsRects(leftTopPoint, widgetSizes, tags, 0 , currentEditIndex);
    CalculateTagOnEdit(leftTopPoint, widgetSizes);
    CalculateTagsRects(leftTopPoint, widgetSizes, tags, currentEditIndex+1, tags.count());
}

void TagsPresenter::CalculateTagsRects(QPoint &leftTopPoint,const QRect &widgetSizes, QVector<Tag> &Tags, int beginTagIndex, int lastTagIndex)
{
    Q_ASSERT(beginTagIndex>=0 && beginTagIndex<=lastTagIndex);
    const int fontMetricsHeight= m_guiWidget->fontMetrics().height();//высота шрифта
    for (int i=beginTagIndex; i<lastTagIndex; ++i)
    {
        const int textMetricsWidth = m_guiWidget->fontMetrics().horizontalAdvance(Tags.at(i).text);//ширина шрифта
        if (leftTopPoint.x()+textMetricsWidth+tag_inner_left_padding+2*tag_inner_right_padding+ tag_cross_spacing + tag_cross_width>widgetSizes.width())//смотрим можем ли поместить все наше добро с отступами, если да то
        {
            leftTopPoint.setX(0);//перенос точки в ноль на след строке
            leftTopPoint.setY(leftTopPoint.y()+tag_inner_bottom_padding+fontMetricsHeight);
        }
        QRect newTagRect(leftTopPoint, QSize(textMetricsWidth, fontMetricsHeight));//прямоугольник тэга
        newTagRect.adjust(tag_inner_left_padding,
                          tag_inner_top_padding,
                          2*tag_inner_right_padding + tag_cross_spacing + tag_cross_width,
                          tag_inner_bottom_padding);//Добавляет соответственно к существующим координатам прямоугольника.
        //Пояснение:!
        //dx1=tag_inner_left_padding - отступ слева что бы место было
        //dy1=tag_inner_top_padding - отступ сверху
        //dx2=2*tag_inner_right_padding + tag_cross_spacing + tag_cross_width 2 - отсупа справа + отсуп крестика + ширина крестика
        //dy2=tag_inner_bottom_padding - отступ снизу
        Tags[i].rect = newTagRect;//записываем в память
        leftTopPoint.setX(newTagRect.right() + tag_horisontal_spacing);//устанавливаем новую точку
    }
}

void TagsPresenter::CalculateTagOnEdit(QPoint &leftTopPoint, const QRect &widgetSizes)
{
    const int fontMetricsHeight= m_guiWidget->fontMetrics().height();//высота шрифта
    const int editedTagHeight=tag_inner_top_padding+ fontMetricsHeight + tag_inner_bottom_padding;
    const int fontMetricsWidth=FONT_METRICS_WIDTH(m_guiWidget->fontMetrics(), m_textLayout->text());
    const int editedTagWidth = fontMetricsWidth +tag_inner_left_padding + tag_inner_right_padding;
    if (leftTopPoint.x() + editedTagWidth < widgetSizes.width())
    {
        SetCurrentEdittedTagRect(QRect(leftTopPoint, QSize(editedTagWidth, editedTagHeight)));
    }
    else
    {
        leftTopPoint.setX(0);
        leftTopPoint.setY(leftTopPoint.y()+2*tag_inner_bottom_padding+fontMetricsHeight+tag_inner_top_padding);
        SetCurrentEdittedTagRect(QRect(leftTopPoint, QSize(editedTagWidth, editedTagHeight)));
    }
    leftTopPoint += QPoint(editedTagWidth + tag_horisontal_spacing, 0);
}

void TagsPresenter::SetCursorVisible(bool visible)
{
    if (m_cursorBlinkTimerId)
    {
        m_guiWidget->killTimer(m_cursorBlinkTimerId);
        m_cursorBlinkTimerId = 0;
        m_cursorBlinkStatus = true;
    }

    if (visible)
    {
        int cursorFlashTime = QGuiApplication::styleHints()->cursorFlashTime();
        if (cursorFlashTime >= 2)
        {
            m_cursorBlinkTimerId = m_guiWidget->startTimer(cursorFlashTime / 2);
        }
    }
    else
    {
        m_cursorBlinkStatus = false;
    }
}

bool TagsPresenter::cursorVisible() const
{
    return m_cursorBlinkTimerId;
}

void TagsPresenter::updateCursorBlinking()
{
    SetCursorVisible(cursorVisible());
}

void TagsPresenter::UpdateDisplayText()
{
    //        MakeLayout();
    m_textLayout->clearLayout();
    m_textLayout->setText(GetCurrentEdittedTagText());
    m_textLayout->beginLayout();
    m_textLayout->createLine();
    m_textLayout->endLayout();
}

void TagsPresenter::setEditingIndex(int index)
{
    Q_ASSERT(index>=0 && index <= tags.count());
    if (GetCurrentEdittedTagText().isEmpty())
    {
        tags.erase(std::next(tags.begin(), std::ptrdiff_t(currentEditIndex)));
        if (currentEditIndex <= index)
        {
            --index;
        }
    }
    currentEditIndex = index;
}

void TagsPresenter::SetCurrentText(const QString &text)
{
    SetTextInEdittedTagText(text);
    MoveCursor(GetCurrentEdittedTagText().count(), 0, false);
    UpdateDisplayText();
    CalculateRects();
    m_guiWidget->update();
}

const QString &TagsPresenter::GetCurrentEdittedTagText() const
{
    return tags.at(currentEditIndex).text;
}

QString &TagsPresenter::SetTextInEdittedTagText(const QString &text)
{
    return tags[currentEditIndex].text=text;
}

QString &TagsPresenter::RemoveCharectersInEdittedTagText(int startPos, int count)
{
    if (startPos>=0 && startPos<tags[currentEditIndex].text.count())
    {
        return tags[currentEditIndex].text.remove(startPos, count);
    }
    else
    {

    }
}

QString &TagsPresenter::InsertCharectersInEdittedTagText(int startPos, const QString &string)
{
    Q_ASSERT(startPos>=0 && startPos<=tags[currentEditIndex].text.count());
    return tags[currentEditIndex].text.insert(startPos, string);
}

const QRect &TagsPresenter::GetCurrentEdittedTagRect() const
{
    return tags.at(currentEditIndex).rect;
}

QRect &TagsPresenter::SetCurrentEdittedTagRect(const QRect &rect)
{
    return tags[currentEditIndex].rect=rect;
}

void TagsPresenter::EditNewTag()
{
    tags.push_back(Tag());
    setEditingIndex(tags.size() - 1);
    MoveCursor(0, 0, false);
}

QVector<QTextLayout::FormatRange> TagsPresenter::formatting() const
{
    if (select_size == 0)
    {
        return {};
    }

    QTextLayout::FormatRange selection;
    selection.start = select_start;
    selection.length = select_size;
    selection.format.setBackground(m_guiWidget->palette().brush(QPalette::Highlight));
    selection.format.setForeground(m_guiWidget->palette().brush(QPalette::HighlightedText));
    return {selection};
}

bool TagsPresenter::hasSelection() const noexcept
{
    return select_size > 0;
}

void TagsPresenter::removeSelection()
{
    m_cursorPosition = select_start;
    RemoveCharectersInEdittedTagText(m_cursorPosition, select_size);
    DeselectAll();
}

void TagsPresenter::removeBackwardOne()
{
    if (hasSelection())
    {
        removeSelection();
    }
    else
    {
        RemoveCharectersInEdittedTagText(--m_cursorPosition, 1);
    }
}

void TagsPresenter::SelectAll()
{
    select_start = 0;
    select_size = GetCurrentEdittedTagText().size();
}

void TagsPresenter::DeselectAll()
{
    select_start = 0;
    select_size = 0;
}

void TagsPresenter::MoveCursor(int posX, int PosY,  bool marked)
{
    if (marked)
    {
        int lastPosition = select_start + select_size;
        int anchor;
        if(select_size>0 && m_cursorPosition==select_size)
        {
            anchor=lastPosition;
        }
        else
        {
            if(select_size > 0 && m_cursorPosition == lastPosition)
            {
                anchor=select_size;
            }
            else
            {
                anchor=m_cursorPosition;
            }
        }
        select_start = qMin(anchor, posX);
        select_size = qMax(anchor, posX) - select_start;
    }
    else
    {
        DeselectAll();
    }
    m_cursorPosition = posX;
}

qreal TagsPresenter::natrualWidth() const
{
    return tags.back().rect.right() - tags.front().rect.left();
}

qreal TagsPresenter::cursorToX()
{
    return m_textLayout->lineAt(0).cursorToX(m_cursorPosition);
}

void TagsPresenter::MakeLayout()
{
    int vertPos=0;
    QTextLine line;
    m_textLayout->setText(GetCurrentEdittedTagText());
    m_textLayout->beginLayout();
    line = m_textLayout->createLine();
    while (line.isValid())
    {
        line.setLineWidth(m_guiWidget->width());
        line.setPosition(QPointF(0, 0));
        vertPos += line.height();
        line = m_textLayout->createLine();
    }
    m_textLayout->endLayout();
}

void TagsPresenter::calcHScroll(const QRect &r)
{
    auto const rect = GetInputWidgetRect();
    auto const width_used = qRound(natrualWidth()) + 1;
    int const cix = r.x() + qRound(cursorToX());
    if (width_used <= rect.width())
    {
        // text fit
        hscroll = 0;
    }
    else
        if (cix - hscroll >= rect.width())
        {
            // text doesn't fit, cursor is to the right of lineRect (scroll right)
            hscroll = cix - rect.width() + 1;
        }
        else
            if (cix - hscroll < 0 && hscroll < width_used)
            {
                // text doesn't fit, cursor is to the left of lineRect (scroll left)
                hscroll = cix;
            }
            else
                if (width_used - hscroll < rect.width())
                {
                    // text doesn't fit, text document is to the left of lineRect; align
                    // right
                    hscroll = width_used - rect.width() + 1;
                }
                else
                {
                    //in case the text is bigger than the lineedit, the hscroll can never be negative
                    hscroll = qMax(0, hscroll);
                }
}

void TagsPresenter::EditPreviousTag()
{
    if (currentEditIndex > 0)
    {
        setEditingIndex(currentEditIndex - 1);
        MoveCursor(GetCurrentEdittedTagText().size(), 0,  false);
    }
}

void TagsPresenter::EditNextTag()
{
    if (currentEditIndex < tags.count() - 1)
    {
        setEditingIndex(currentEditIndex + 1);
        MoveCursor(0, 0, false);
    }
}

void TagsPresenter::SetTagEditableAtIndex(int i)
{
    assert(i >= 0 && i < tags.count());
    setEditingIndex(i);
    MoveCursor(GetCurrentEdittedTagText().size(), 0,  false);
}





