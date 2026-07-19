#include "main.h"
#include "util.h"
#include "world.h"
#include "menu.h"
#include "actions.h"
#include "sound.h"
#include <shlwapi.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <stdarg.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 860
#define MARGIN 14
#define BANNER_HEIGHT 64
#define CAPTION_HEIGHT 22
#define LOG_WIDTH_PERCENT 60
#define STATUS_HEIGHT_PERCENT 33
#define FAMILY_HEIGHT 64
#define BUTTON_HEIGHT 34
#define STAR_SIZE 15
#define STAR_SPACING 38
#define STAR_LEFT_PAD 26
#define STAR_TOP_PAD 26
#define FIGURE_LEFT_PAD 134
#define FIGURE_SPACING 34
#define FIGURE_TOP_PAD 10
#define FIGURE_CHILD_DROP 11
#define FIGURE_SCALE_ADULT 100
#define FIGURE_SCALE_CHILD 68
#define FONT_HEIGHT 16
#define BANNER_FONT_HEIGHT 32
#define CAPTION_FONT_HEIGHT 14
#define CHOICE_ITEM_HEIGHT 24
#define PINSTRIPE_SPACING 26
#define FRAME_GAP 4
#define CORNER_TICK 9
#define EDIT_PADDING 8
#define LOG_LIMIT_CHARS 0x7FFFFFFE
#define LOG_APPEND_POSITION 0x7FFFFFF0
#define ICON_RESOURCE_ID 100
#define ID_CHOICES 103
#define ID_BUTTON 104
#define ID_MAPBTN 105
#define MAP_HEIGHT_PERCENT 40
#define MAP_MAX_HEIGHT 330
#define MAP_BUTTON_WIDTH 110
#define MAP_CELL_PAD 5
#define MAP_PIP_SIZE 6
#define MAP_PIP_STEP 9
#define MAP_STREET_STEP 26
#define MAP_BIZ_MARK 7
#define MAP_BIZ_STEP 10
#define PING_SIZE 13
#define SMALL_FONT_HEIGHT 13
#define COLOR_POLICE RGB(96, 118, 152)

#define COLOR_BG RGB(17, 17, 21)
#define COLOR_STRIPE RGB(26, 25, 31)
#define COLOR_PANE RGB(29, 28, 34)
#define COLOR_SELECT RGB(48, 42, 34)
#define COLOR_TEXT RGB(216, 209, 191)
#define COLOR_GOLD RGB(205, 165, 66)
#define COLOR_GOLD_DIM RGB(120, 99, 54)
#define COLOR_BLOOD RGB(190, 84, 74)

static const WCHAR* const CLASS_NAME = L"GanglandMainWindow";
static const WCHAR* const WINDOW_TITLE = L"Gangland - Paradise City";
static const WCHAR* const BANNER_TEXT = L"G A N G L A N D";
static const WCHAR* const BANNER_SUB = L"PARADISE CITY, 1932  -  THIS FAMILY MAY JUST BE THE DEATH OF YOU";
static const WCHAR* const CAPTION_LOG = L"THE STREET";
static const WCHAR* const CAPTION_STATUS = L"THE FAMILY LEDGER";
static const WCHAR* const CAPTION_CHOICES = L"YOUR MOVE";
static const WCHAR* const CAPTION_FAMILY = L"LA FAMIGLIA";

static const POINT STAR_SHAPE[10] = { { 0, -100 }, { 24, -32 }, { 95, -31 }, { 38, 12 }, { 59, 81 }, { 0, 40 }, { -59, 81 }, { -38, 12 }, { -95, -31 }, { -24, -32 } };

static const WCHAR* const CAPTION_MAP = L"PARADISE CITY";
static const RECT MAP_CELLS[NUM_DISTRICTS] = { { 0, 500, 300, 1000 }, { 0, 0, 550, 500 }, { 550, 0, 1000, 500 }, { 300, 500, 620, 1000 }, { 620, 500, 1000, 1000 } };
static const WCHAR* const MAP_NAMES[NUM_DISTRICTS] = { L"LITTLE ITALY", L"DOWNTOWN", L"THE DOCKS", L"MARKET QUARTER", L"THE SUBURBS" };
static const WCHAR* const MAP_HOODS_A[NUM_DISTRICTS] = { L"Via Roma", L"Theatre Row", L"Cannery Wharf", L"Grand Bazaar", L"Elm Rows" };
static const WCHAR* const MAP_HOODS_B[NUM_DISTRICTS] = { L"the Old Quarter", L"City Hall steps", L"Pier 13", L"Butcher's Row", L"Orchard Lane" };
static const POINT BURST_SHAPE[16] = { { 100, 0 }, { 42, 17 }, { 71, 71 }, { 17, 42 }, { 0, 100 }, { -17, 42 }, { -71, 71 }, { -42, 17 }, { -100, 0 }, { -42, -17 }, { -71, -71 }, { -17, -42 }, { 0, -100 }, { 17, -42 }, { 71, -71 }, { 42, -17 } };

static void LayoutControls(App* app)
{
    RECT client = { 0 };
    int contentTop = 0;
    int logWidth = 0;
    int rightLeft = 0;
    int rightWidth = 0;
    int paneTop = 0;
    int mapHeight = 0;
    int logTop = 0;
    int logHeight = 0;
    int statusHeight = 0;
    int familyTop = 0;
    int choicesTop = 0;
    int choicesHeight = 0;
    int buttonTop = 0;

    if (NULL == app || NULL == app->windowMain)
    {
        goto Cleanup;
    }
    GetClientRect(app->windowMain, &client);
    contentTop = MARGIN + BANNER_HEIGHT;
    logWidth = ((client.right - (MARGIN * 3)) * LOG_WIDTH_PERCENT) / 100;
    rightLeft = (MARGIN * 2) + logWidth;
    rightWidth = client.right - rightLeft - MARGIN;
    paneTop = contentTop + CAPTION_HEIGHT;
    mapHeight = ((client.bottom - contentTop) * MAP_HEIGHT_PERCENT) / 100;
    if (MAP_MAX_HEIGHT < mapHeight)
    {
        mapHeight = MAP_MAX_HEIGHT;
    }
    if (0 != app->game->mapCollapsed)
    {
        mapHeight = 0;
    }
    SetRect(&app->rectMap, MARGIN, paneTop, MARGIN + logWidth, paneTop + mapHeight);
    logTop = (0 == mapHeight) ? paneTop : (paneTop + mapHeight + MARGIN + CAPTION_HEIGHT);
    logHeight = client.bottom - logTop - MARGIN;
    MoveWindow(app->windowMapButton, MARGIN + logWidth - MAP_BUTTON_WIDTH, contentTop - 2, MAP_BUTTON_WIDTH, CAPTION_HEIGHT, TRUE);
    statusHeight = ((client.bottom - contentTop - (MARGIN * 2)) * STATUS_HEIGHT_PERCENT) / 100;
    familyTop = paneTop + statusHeight + MARGIN + CAPTION_HEIGHT;
    choicesTop = familyTop + FAMILY_HEIGHT + MARGIN + CAPTION_HEIGHT;
    choicesHeight = client.bottom - choicesTop - BUTTON_HEIGHT - (MARGIN * 2);
    buttonTop = choicesTop + choicesHeight + MARGIN;
    MoveWindow(app->windowLog, MARGIN, logTop, logWidth, logHeight, TRUE);
    MoveWindow(app->windowStatus, rightLeft, paneTop, rightWidth, statusHeight, TRUE);
    MoveWindow(app->windowChoices, rightLeft, choicesTop, rightWidth, choicesHeight, TRUE);
    MoveWindow(app->windowButton, rightLeft, buttonTop, rightWidth, BUTTON_HEIGHT, TRUE);
    SetRect(&app->rectLog, MARGIN, logTop, MARGIN + logWidth, logTop + logHeight);
    SetRect(&app->rectStatus, rightLeft, paneTop, rightLeft + rightWidth, paneTop + statusHeight);
    SetRect(&app->rectFamily, rightLeft, familyTop, rightLeft + rightWidth, familyTop + FAMILY_HEIGHT);
    SetRect(&app->rectChoices, rightLeft, choicesTop, rightLeft + rightWidth, choicesTop + choicesHeight);
    InvalidateRect(app->windowMain, NULL, TRUE);

Cleanup:

    return;
}

static HWND MakeChild(App* app, const WCHAR* kind, DWORD style)
{
    HWND child = NULL;

    child = CreateWindowExW(0, kind, L"", WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10, app->windowMain, NULL, GetModuleHandleW(NULL), NULL);
    if (NULL != child && NULL != app->font)
    {
        SendMessageW(child, WM_SETFONT, (WPARAM)app->font, TRUE);
    }

    return child;
}

static void CreateControls(App* app)
{
    app->font = CreateFontW(FONT_HEIGHT, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
    app->fontBanner = CreateFontW(BANNER_FONT_HEIGHT, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_ROMAN, L"Georgia");
    app->fontCaption = CreateFontW(CAPTION_FONT_HEIGHT, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_ROMAN, L"Georgia");
    app->fontSmall = CreateFontW(SMALL_FONT_HEIGHT, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_ROMAN, L"Georgia");
    app->brushBackground = CreateSolidBrush(COLOR_BG);
    app->brushPane = CreateSolidBrush(COLOR_PANE);
    app->brushSelect = CreateSolidBrush(COLOR_SELECT);
    app->brushGold = CreateSolidBrush(COLOR_GOLD);
    app->brushBlood = CreateSolidBrush(COLOR_BLOOD);
    app->brushText = CreateSolidBrush(COLOR_TEXT);
    app->brushPolice = CreateSolidBrush(COLOR_POLICE);
    app->penGold = CreatePen(PS_SOLID, 1, COLOR_GOLD);
    app->penGoldDim = CreatePen(PS_SOLID, 1, COLOR_GOLD_DIM);
    app->penStripe = CreatePen(PS_SOLID, 1, COLOR_STRIPE);
    app->penBlood = CreatePen(PS_SOLID, 1, COLOR_BLOOD);
    app->penPolice = CreatePen(PS_SOLID, 1, COLOR_POLICE);
    app->windowLog = MakeChild(app, L"EDIT", ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL);
    app->windowStatus = MakeChild(app, L"EDIT", ES_MULTILINE | ES_READONLY);
    app->windowChoices = MakeChild(app, L"LISTBOX", LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | WS_VSCROLL);
    app->windowButton = MakeChild(app, L"BUTTON", BS_OWNERDRAW);
    app->windowMapButton = MakeChild(app, L"BUTTON", BS_OWNERDRAW);
    SetWindowLongPtrW(app->windowChoices, GWLP_ID, ID_CHOICES);
    SetWindowLongPtrW(app->windowButton, GWLP_ID, ID_BUTTON);
    SetWindowLongPtrW(app->windowMapButton, GWLP_ID, ID_MAPBTN);
    SetWindowTextW(app->windowButton, L"C H O O S E");
    SetWindowTextW(app->windowMapButton, L"HIDE MAP");
    SendMessageW(app->windowLog, EM_SETLIMITTEXT, LOG_LIMIT_CHARS, 0);
    SendMessageW(app->windowLog, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(EDIT_PADDING, EDIT_PADDING));
    SendMessageW(app->windowStatus, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(EDIT_PADDING, EDIT_PADDING));
    SetWindowTheme(app->windowLog, L"DarkMode_Explorer", NULL);
    SetWindowTheme(app->windowChoices, L"DarkMode_Explorer", NULL);
}

void UiLog(App* app, const WCHAR* text)
{
    if (NULL == app || NULL == text)
    {
        goto Cleanup;
    }
    SendMessageW(app->windowLog, EM_SETSEL, (WPARAM)LOG_APPEND_POSITION, (LPARAM)LOG_APPEND_POSITION);
    SendMessageW(app->windowLog, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessageW(app->windowLog, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");

Cleanup:

    return;
}

void UiLogFmt(App* app, const WCHAR* format, ...)
{
    va_list args = NULL;

    if (NULL == app || NULL == app->formatBuffer)
    {
        goto Cleanup;
    }
    va_start(args, format);
    wvnsprintfW(app->formatBuffer, FORMAT_BUFFER_CHARS, format, args);
    va_end(args);
    UiLog(app, app->formatBuffer);

Cleanup:

    return;
}

void AddChoice(App* app, const WCHAR* label, int packedAction)
{
    if (NULL == app || MAX_CHOICES <= app->choiceCount)
    {
        goto Cleanup;
    }
    SendMessageW(app->windowChoices, LB_ADDSTRING, 0, (LPARAM)label);
    app->choiceActions[app->choiceCount] = packedAction;
    app->choiceCount = app->choiceCount + 1;

Cleanup:

    return;
}

void RefreshUi(App* app)
{
    if (NULL == app || NULL == app->game)
    {
        goto Cleanup;
    }
    BuildStatusText(app);
    SetWindowTextW(app->windowMapButton, (0 != app->game->mapCollapsed) ? L"SHOW MAP" : L"HIDE MAP");
    if ((0 != app->game->mapCollapsed) != (0 == (app->rectMap.bottom - app->rectMap.top)))
    {
        LayoutControls(app);
    }
    SendMessageW(app->windowChoices, WM_SETREDRAW, FALSE, 0);
    SendMessageW(app->windowChoices, LB_RESETCONTENT, 0, 0);
    app->choiceCount = 0;
    BuildChoices(app);
    SendMessageW(app->windowChoices, LB_SETCURSEL, 0, 0);
    SendMessageW(app->windowChoices, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(app->windowChoices, NULL, TRUE);
    InvalidateRect(app->windowMain, &app->rectFamily, TRUE);
    if (0 == app->game->mapCollapsed)
    {
        InvalidateRect(app->windowMain, &app->rectMap, TRUE);
    }

Cleanup:

    return;
}

static void OnChoose(App* app)
{
    LRESULT selected = 0;

    if (NULL == app)
    {
        goto Cleanup;
    }
    selected = SendMessageW(app->windowChoices, LB_GETCURSEL, 0, 0);
    if (LB_ERR == selected || app->choiceCount <= (int)selected)
    {
        goto Cleanup;
    }
    HandleAction(app, app->choiceActions[selected]);
    RefreshUi(app);

Cleanup:

    return;
}

static void PaintBackground(App* app, HDC dc)
{
    RECT client = { 0 };
    int x = 0;

    GetClientRect(app->windowMain, &client);
    FillRect(dc, &client, app->brushBackground);
    SelectObject(dc, app->penStripe);
    for (x = MARGIN; client.right > x; x = x + PINSTRIPE_SPACING)
    {
        MoveToEx(dc, x, 0, NULL);
        LineTo(dc, x, client.bottom);
    }
}

static void PaintBanner(App* app, HDC dc)
{
    RECT client = { 0 };
    RECT textRect = { 0 };
    int lineY = 0;

    GetClientRect(app->windowMain, &client);
    SetBkMode(dc, TRANSPARENT);
    SelectObject(dc, app->fontBanner);
    SetTextColor(dc, COLOR_GOLD);
    SetRect(&textRect, 0, MARGIN - 4, client.right, MARGIN + BANNER_FONT_HEIGHT);
    DrawTextW(dc, BANNER_TEXT, -1, &textRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
    SelectObject(dc, app->fontCaption);
    SetTextColor(dc, COLOR_GOLD_DIM);
    SetRect(&textRect, 0, MARGIN + BANNER_FONT_HEIGHT + 2, client.right, MARGIN + BANNER_HEIGHT);
    DrawTextW(dc, BANNER_SUB, -1, &textRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
    lineY = MARGIN + BANNER_HEIGHT - 6;
    SelectObject(dc, app->penGoldDim);
    MoveToEx(dc, MARGIN, lineY, NULL);
    LineTo(dc, client.right - MARGIN, lineY);
    SelectObject(dc, app->penGold);
    MoveToEx(dc, client.right / 4, lineY + 2, NULL);
    LineTo(dc, (client.right * 3) / 4, lineY + 2);
}

static void PaintCorners(HDC dc, const RECT* frame, HPEN pen)
{
    SelectObject(dc, pen);
    MoveToEx(dc, frame->left, frame->top + CORNER_TICK, NULL);
    LineTo(dc, frame->left, frame->top);
    LineTo(dc, frame->left + CORNER_TICK, frame->top);
    MoveToEx(dc, frame->right - CORNER_TICK, frame->top, NULL);
    LineTo(dc, frame->right, frame->top);
    LineTo(dc, frame->right, frame->top + CORNER_TICK);
    MoveToEx(dc, frame->right, frame->bottom - CORNER_TICK, NULL);
    LineTo(dc, frame->right, frame->bottom);
    LineTo(dc, frame->right - CORNER_TICK, frame->bottom);
    MoveToEx(dc, frame->left + CORNER_TICK, frame->bottom, NULL);
    LineTo(dc, frame->left, frame->bottom);
    LineTo(dc, frame->left, frame->bottom - CORNER_TICK);
}

static void PaintPane(App* app, HDC dc, int which)
{
    const RECT* pane = NULL;
    const WCHAR* caption = NULL;
    RECT frame = { 0 };
    RECT captionRect = { 0 };

    pane = &app->rectLog;
    caption = CAPTION_LOG;
    if (1 == which)
    {
        pane = &app->rectStatus;
        caption = CAPTION_STATUS;
    }
    if (2 == which)
    {
        pane = &app->rectChoices;
        caption = CAPTION_CHOICES;
    }
    if (3 == which)
    {
        pane = &app->rectFamily;
        caption = CAPTION_FAMILY;
        FillRect(dc, &app->rectFamily, app->brushPane);
    }
    if (4 == which)
    {
        pane = &app->rectMap;
        caption = CAPTION_MAP;
    }
    frame = *pane;
    InflateRect(&frame, FRAME_GAP, FRAME_GAP);
    SelectObject(dc, app->penGoldDim);
    SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(dc, frame.left, frame.top, frame.right, frame.bottom);
    PaintCorners(dc, &frame, app->penGold);
    SetBkMode(dc, TRANSPARENT);
    SelectObject(dc, app->fontCaption);
    SetTextColor(dc, COLOR_GOLD_DIM);
    SetRect(&captionRect, pane->left + 2, frame.top - CAPTION_HEIGHT + 2, pane->right, frame.top - 2);
    DrawTextW(dc, caption, -1, &captionRect, DT_LEFT | DT_TOP | DT_SINGLELINE);
}

static void HandleDrawList(App* app, const DRAWITEMSTRUCT* item)
{
    WCHAR* buffer = NULL;
    LRESULT length = 0;
    RECT textRect = { 0 };
    RECT bar = { 0 };
    COLORREF color = COLOR_TEXT;
    int selected = 0;

    selected = (0 != (item->itemState & ODS_SELECTED)) ? 1 : 0;
    FillRect(item->hDC, &item->rcItem, (0 != selected) ? app->brushSelect : app->brushPane);
    if (0 > (int)item->itemID)
    {
        goto Cleanup;
    }
    length = SendMessageW(item->hwndItem, LB_GETTEXTLEN, item->itemID, 0);
    if (LB_ERR == length || 0 > length)
    {
        goto Cleanup;
    }
    buffer = (WCHAR*)AllocZeroed(((SIZE_T)length + 1) * sizeof(WCHAR));
    if (NULL == buffer)
    {
        goto Cleanup;
    }
    SendMessageW(item->hwndItem, LB_GETTEXT, item->itemID, (LPARAM)buffer);
    if (L'[' == buffer[0])
    {
        color = COLOR_BLOOD;
    }
    if (0 != selected)
    {
        color = COLOR_GOLD;
        bar = item->rcItem;
        bar.right = bar.left + 3;
        FillRect(item->hDC, &bar, app->brushSelect);
        SelectObject(item->hDC, app->penGold);
        MoveToEx(item->hDC, bar.left + 1, bar.top + 3, NULL);
        LineTo(item->hDC, bar.left + 1, bar.bottom - 3);
    }
    SetBkMode(item->hDC, TRANSPARENT);
    SetTextColor(item->hDC, color);
    SelectObject(item->hDC, app->font);
    textRect = item->rcItem;
    textRect.left = textRect.left + EDIT_PADDING + 2;
    DrawTextW(item->hDC, buffer, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

Cleanup:
    FreeMemory(buffer);

    return;
}

static void HandleDrawButton(App* app, const DRAWITEMSTRUCT* item)
{
    WCHAR* buffer = NULL;
    RECT inner = { 0 };
    int pressed = 0;

    buffer = (WCHAR*)AllocZeroed(FORMAT_BUFFER_CHARS * sizeof(WCHAR));
    if (NULL == buffer)
    {
        goto Cleanup;
    }
    pressed = (0 != (item->itemState & ODS_SELECTED)) ? 1 : 0;
    FillRect(item->hDC, &item->rcItem, (0 != pressed) ? app->brushSelect : app->brushPane);
    SelectObject(item->hDC, app->penGold);
    SelectObject(item->hDC, GetStockObject(HOLLOW_BRUSH));
    Rectangle(item->hDC, item->rcItem.left, item->rcItem.top, item->rcItem.right, item->rcItem.bottom);
    GetWindowTextW(item->hwndItem, buffer, FORMAT_BUFFER_CHARS);
    SetBkMode(item->hDC, TRANSPARENT);
    SetTextColor(item->hDC, COLOR_GOLD);
    SelectObject(item->hDC, app->fontCaption);
    inner = item->rcItem;
    if (0 != pressed)
    {
        OffsetRect(&inner, 1, 1);
    }
    DrawTextW(item->hDC, buffer, -1, &inner, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

Cleanup:
    FreeMemory(buffer);

    return;
}

static void DrawStar(HDC dc, const POINT* center, int size)
{
    POINT points[10] = { 0 };
    int index = 0;

    for (index = 0; 10 > index; index++)
    {
        points[index].x = center->x + ((STAR_SHAPE[index].x * size) / 100);
        points[index].y = center->y + ((STAR_SHAPE[index].y * size) / 100);
    }
    Polygon(dc, points, 10);
}

static int Scaled(int scale, int units)
{
    int result = 0;

    result = (units * scale) / 100;

    return result;
}

static void DrawWoman(HDC dc, const POINT* origin, int scale)
{
    POINT dress[6] = { 0 };
    int x = 0;
    int y = 0;

    x = origin->x;
    y = origin->y;
    dress[0].x = x - Scaled(scale,6);
    dress[0].y = y + Scaled(scale,15);
    dress[1].x = x + Scaled(scale,6);
    dress[1].y = y + Scaled(scale,15);
    dress[2].x = x + Scaled(scale,4);
    dress[2].y = y + Scaled(scale,23);
    dress[3].x = x + Scaled(scale,10);
    dress[3].y = y + Scaled(scale,38);
    dress[4].x = x - Scaled(scale,10);
    dress[4].y = y + Scaled(scale,38);
    dress[5].x = x - Scaled(scale,4);
    dress[5].y = y + Scaled(scale,23);
    Polygon(dc, dress, 6);
    Ellipse(dc, x - Scaled(scale,5), y + Scaled(scale,4), x + Scaled(scale,5), y + Scaled(scale,14));
    Ellipse(dc, x - Scaled(scale,8), y + Scaled(scale,2), x + Scaled(scale,8), y + Scaled(scale,7));
    Ellipse(dc, x - Scaled(scale,5), y - Scaled(scale,3), x + Scaled(scale,5), y + Scaled(scale,4));
}

static void DrawMan(HDC dc, const POINT* origin, int scale)
{
    POINT suit[4] = { 0 };
    int x = 0;
    int y = 0;

    x = origin->x;
    y = origin->y;
    suit[0].x = x - Scaled(scale,7);
    suit[0].y = y + Scaled(scale,15);
    suit[1].x = x + Scaled(scale,7);
    suit[1].y = y + Scaled(scale,15);
    suit[2].x = x + Scaled(scale,5);
    suit[2].y = y + Scaled(scale,30);
    suit[3].x = x - Scaled(scale,5);
    suit[3].y = y + Scaled(scale,30);
    Polygon(dc, suit, 4);
    Rectangle(dc, x - Scaled(scale,5), y + Scaled(scale,30), x - Scaled(scale,1), y + Scaled(scale,38));
    Rectangle(dc, x + Scaled(scale,1), y + Scaled(scale,30), x + Scaled(scale,5), y + Scaled(scale,38));
    Ellipse(dc, x - Scaled(scale,5), y + Scaled(scale,4), x + Scaled(scale,5), y + Scaled(scale,14));
    Ellipse(dc, x - Scaled(scale,9), y + Scaled(scale,3), x + Scaled(scale,9), y + Scaled(scale,7));
    Rectangle(dc, x - Scaled(scale,5), y - Scaled(scale,3), x + Scaled(scale,5), y + Scaled(scale,5));
}

static void GetSlotOrigin(App* app, int slot, POINT* origin)
{
    origin->x = app->rectFamily.left + FIGURE_LEFT_PAD + (slot * FIGURE_SPACING);
    origin->y = app->rectFamily.top + FIGURE_TOP_PAD;
}

static void SelectHeirColors(App* app, HDC dc, int heirType)
{
    SelectObject(dc, app->penGoldDim);
    SelectObject(dc, app->brushGold);
    if (HEIR_LAWYER == heirType)
    {
        SelectObject(dc, app->brushText);
    }
    if (HEIR_SEDUCTRESS == heirType)
    {
        SelectObject(dc, app->brushBlood);
    }
}

static void PaintFamily(App* app, HDC dc)
{
    GameState* game = NULL;
    POINT starCenter = { 0 };
    POINT figureOrigin = { 0 };
    RECT emptyRect = { 0 };
    int index = 0;
    int slot = 0;
    int filledStars = 0;
    int anybody = 0;

    game = app->game;
    filledStars = game->rank + 1;
    starCenter.y = app->rectFamily.top + STAR_TOP_PAD;
    for (index = 0; 3 > index; index++)
    {
        starCenter.x = app->rectFamily.left + STAR_LEFT_PAD + (index * STAR_SPACING);
        if (filledStars > index)
        {
            SelectObject(dc, app->penGold);
            SelectObject(dc, app->brushGold);
        }
        else
        {
            SelectObject(dc, app->penGoldDim);
            SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
        }
        DrawStar(dc, &starCenter, STAR_SIZE);
    }
    SelectObject(dc, app->penGoldDim);
    MoveToEx(dc, app->rectFamily.left + FIGURE_LEFT_PAD - 16, app->rectFamily.top + 8, NULL);
    LineTo(dc, app->rectFamily.left + FIGURE_LEFT_PAD - 16, app->rectFamily.bottom - 8);
    if (WIFE_MARRIED == game->wife.status || WIFE_ENGAGED == game->wife.status)
    {
        anybody = 1;
        SelectObject(dc, (KIDNAP_WIFE == game->kidnapVictim) ? app->penBlood : app->penGold);
        SelectObject(dc, (KIDNAP_WIFE == game->kidnapVictim) ? GetStockObject(HOLLOW_BRUSH) : app->brushGold);
        if (WIFE_ENGAGED == game->wife.status)
        {
            SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
        }
        GetSlotOrigin(app, slot, &figureOrigin);
        DrawWoman(dc, &figureOrigin, FIGURE_SCALE_ADULT);
        slot = slot + 1;
    }
    if (0 != game->mistress)
    {
        anybody = 1;
        SelectObject(dc, app->penBlood);
        SelectObject(dc, app->brushBlood);
        GetSlotOrigin(app, slot, &figureOrigin);
        DrawWoman(dc, &figureOrigin, FIGURE_SCALE_ADULT);
        slot = slot + 1;
    }
    for (index = 0; MAX_HEIRS > index; index++)
    {
        if (0 == game->heirs[index].exists)
        {
            continue;
        }
        anybody = 1;
        SelectHeirColors(app, dc, game->heirs[index].type);
        if (index == game->kidnapVictim)
        {
            SelectObject(dc, app->penBlood);
            SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
        }
        GetSlotOrigin(app, slot, &figureOrigin);
        figureOrigin.y = figureOrigin.y + FIGURE_CHILD_DROP;
        if (HEIR_ENFORCER == game->heirs[index].type)
        {
            DrawMan(dc, &figureOrigin, FIGURE_SCALE_CHILD);
        }
        else
        {
            DrawWoman(dc, &figureOrigin, FIGURE_SCALE_CHILD);
        }
        slot = slot + 1;
    }
    if (0 == anybody)
    {
        SetBkMode(dc, TRANSPARENT);
        SelectObject(dc, app->fontCaption);
        SetTextColor(dc, COLOR_GOLD_DIM);
        SetRect(&emptyRect, app->rectFamily.left + FIGURE_LEFT_PAD - 6, app->rectFamily.top + 14, app->rectFamily.right - 4, app->rectFamily.bottom);
        DrawTextW(dc, L"No family yet. Power first - then love.", -1, &emptyRect, DT_LEFT | DT_WORDBREAK);
    }
}

static void MapCellRect(App* app, int district, RECT* cell)
{
    int width = 0;
    int height = 0;

    width = app->rectMap.right - app->rectMap.left;
    height = app->rectMap.bottom - app->rectMap.top;
    cell->left = app->rectMap.left + ((MAP_CELLS[district].left * width) / 1000);
    cell->top = app->rectMap.top + ((MAP_CELLS[district].top * height) / 1000);
    cell->right = app->rectMap.left + ((MAP_CELLS[district].right * width) / 1000);
    cell->bottom = app->rectMap.top + ((MAP_CELLS[district].bottom * height) / 1000);
}

static void DrawStreets(App* app, HDC dc, const RECT* cell)
{
    int x = 0;
    int y = 0;

    SelectObject(dc, app->penStripe);
    for (x = cell->left + MAP_STREET_STEP; cell->right - 2 > x; x = x + MAP_STREET_STEP)
    {
        MoveToEx(dc, x, cell->top + 1, NULL);
        LineTo(dc, x, cell->bottom - 1);
    }
    for (y = cell->top + MAP_STREET_STEP; cell->bottom - 2 > y; y = y + MAP_STREET_STEP)
    {
        MoveToEx(dc, cell->left + 1, y, NULL);
        LineTo(dc, cell->right - 1, y);
    }
}

static void DrawHarborEdge(App* app, HDC dc, const RECT* cell)
{
    int y = 0;
    int sway = 0;

    SelectObject(dc, app->penPolice);
    for (y = cell->top + 10; cell->bottom - 8 > y; y = y + 9)
    {
        sway = ((y / 9) % 2) * 4;
        MoveToEx(dc, cell->right - 18 + sway, y, NULL);
        LineTo(dc, cell->right - 8 + sway, y);
    }
}

static void DrawBurst(HDC dc, const POINT* center, int size)
{
    POINT points[16] = { 0 };
    int index = 0;

    for (index = 0; 16 > index; index++)
    {
        points[index].x = center->x + ((BURST_SHAPE[index].x * size) / 100);
        points[index].y = center->y + ((BURST_SHAPE[index].y * size) / 100);
    }
    Polygon(dc, points, 16);
}

static void DrawHeart(HDC dc, const POINT* center, int size)
{
    POINT tip[3] = { 0 };

    Ellipse(dc, center->x - size, center->y - size, center->x + 1, center->y + 1);
    Ellipse(dc, center->x - 1, center->y - size, center->x + size, center->y + 1);
    tip[0].x = center->x - size;
    tip[0].y = center->y - (size / 4);
    tip[1].x = center->x + size;
    tip[1].y = center->y - (size / 4);
    tip[2].x = center->x;
    tip[2].y = center->y + size;
    Polygon(dc, tip, 3);
}

typedef struct PingDraw
{
    POINT center;
    int kind;
} PingDraw;

static void DrawPingMark(App* app, HDC dc, const PingDraw* mark)
{
    const POINT* center = NULL;
    POINT triangle[3] = { 0 };
    POINT diamond[4] = { 0 };
    RECT label = { 0 };
    int ping = 0;

    center = &mark->center;
    ping = mark->kind;
    SetRect(&label, center->x - PING_SIZE, center->y - PING_SIZE, center->x + PING_SIZE, center->y + PING_SIZE);
    if (PING_FIGHT == ping)
    {
        SelectObject(dc, app->penBlood);
        SelectObject(dc, app->brushBlood);
        DrawBurst(dc, center, PING_SIZE);
        goto Cleanup;
    }
    if (PING_MONEY == ping)
    {
        SelectObject(dc, app->penGoldDim);
        SelectObject(dc, app->brushGold);
        Ellipse(dc, center->x - PING_SIZE + 2, center->y - PING_SIZE + 2, center->x + PING_SIZE - 2, center->y + PING_SIZE - 2);
        SelectObject(dc, app->fontCaption);
        SetTextColor(dc, COLOR_BG);
        DrawTextW(dc, L"$", -1, &label, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        goto Cleanup;
    }
    if (PING_ALERT == ping)
    {
        SelectObject(dc, app->penBlood);
        SelectObject(dc, app->brushBlood);
        triangle[0].x = center->x;
        triangle[0].y = center->y - PING_SIZE;
        triangle[1].x = center->x + PING_SIZE;
        triangle[1].y = center->y + ((PING_SIZE * 3) / 4);
        triangle[2].x = center->x - PING_SIZE;
        triangle[2].y = center->y + ((PING_SIZE * 3) / 4);
        Polygon(dc, triangle, 3);
        SelectObject(dc, app->fontCaption);
        SetTextColor(dc, COLOR_BG);
        DrawTextW(dc, L"!", -1, &label, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        goto Cleanup;
    }
    if (PING_EVENT == ping)
    {
        SelectObject(dc, app->penGoldDim);
        SelectObject(dc, app->brushGold);
        diamond[0].x = center->x;
        diamond[0].y = center->y - PING_SIZE;
        diamond[1].x = center->x + PING_SIZE;
        diamond[1].y = center->y;
        diamond[2].x = center->x;
        diamond[2].y = center->y + PING_SIZE;
        diamond[3].x = center->x - PING_SIZE;
        diamond[3].y = center->y;
        Polygon(dc, diamond, 4);
        SelectObject(dc, app->fontCaption);
        SetTextColor(dc, COLOR_BG);
        DrawTextW(dc, L"!", -1, &label, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        goto Cleanup;
    }
    if (PING_FAMILY == ping)
    {
        SelectObject(dc, app->penBlood);
        SelectObject(dc, app->brushBlood);
        DrawHeart(dc, center, PING_SIZE - 3);
    }

Cleanup:

    return;
}

static void DrawHouseIcon(App* app, HDC dc, const RECT* cell)
{
    POINT roof[3] = { 0 };

    SelectObject(dc, app->penGold);
    SelectObject(dc, app->brushGold);
    Rectangle(dc, cell->right - 22, cell->top + 10, cell->right - 8, cell->top + 20);
    roof[0].x = cell->right - 24;
    roof[0].y = cell->top + 10;
    roof[1].x = cell->right - 15;
    roof[1].y = cell->top + 3;
    roof[2].x = cell->right - 6;
    roof[2].y = cell->top + 10;
    Polygon(dc, roof, 3);
}

static void DrawChurchIcon(App* app, HDC dc, const RECT* cell)
{
    POINT roof[3] = { 0 };

    SelectObject(dc, app->penGoldDim);
    SelectObject(dc, app->brushText);
    Rectangle(dc, cell->right - 22, cell->top + 14, cell->right - 8, cell->top + 23);
    roof[0].x = cell->right - 23;
    roof[0].y = cell->top + 14;
    roof[1].x = cell->right - 15;
    roof[1].y = cell->top + 8;
    roof[2].x = cell->right - 7;
    roof[2].y = cell->top + 14;
    Polygon(dc, roof, 3);
    SelectObject(dc, app->penGold);
    MoveToEx(dc, cell->right - 15, cell->top + 8, NULL);
    LineTo(dc, cell->right - 15, cell->top + 2);
    MoveToEx(dc, cell->right - 18, cell->top + 4, NULL);
    LineTo(dc, cell->right - 12, cell->top + 4);
}

static void DrawBizMarks(App* app, HDC dc, int district)
{
    GameState* game = NULL;
    RECT cell = { 0 };
    int index = 0;
    int x = 0;
    int y = 0;

    game = app->game;
    MapCellRect(app, district, &cell);
    x = cell.right - MAP_CELL_PAD - MAP_BIZ_MARK;
    y = cell.bottom - MAP_CELL_PAD - MAP_BIZ_MARK;
    for (index = 0; MAX_BUSINESSES > index; index++)
    {
        if (game->businesses[index].district != district)
        {
            continue;
        }
        if (BIZ_OWNED != game->businesses[index].status && BIZ_EXTORTED != game->businesses[index].status)
        {
            continue;
        }
        SelectObject(dc, app->penGold);
        SelectObject(dc, (BIZ_OWNED == game->businesses[index].status) ? app->brushGold : GetStockObject(HOLLOW_BRUSH));
        Rectangle(dc, x - MAP_BIZ_MARK, y, x, y + MAP_BIZ_MARK);
        x = x - MAP_BIZ_STEP;
    }
}

static void DrawMapCell(App* app, HDC dc, int district)
{
    GameState* game = NULL;
    Rival* rival = NULL;
    RECT cell = { 0 };
    RECT text = { 0 };
    PingDraw pingMark = { 0 };
    int atWar = 0;
    int pipIndex = 0;

    game = app->game;
    MapCellRect(app, district, &cell);
    rival = RivalInDistrict(game, district);
    atWar = (NULL != rival && RIVAL_ANGER_WAR <= rival->anger) ? 1 : 0;
    FillRect(dc, &cell, (district == game->location) ? app->brushSelect : app->brushPane);
    DrawStreets(app, dc, &cell);
    if (DISTRICT_DOCKS == district)
    {
        DrawHarborEdge(app, dc, &cell);
    }
    SelectObject(dc, (0 != atWar) ? app->penBlood : app->penGoldDim);
    SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(dc, cell.left, cell.top, cell.right, cell.bottom);
    SetBkMode(dc, TRANSPARENT);
    SelectObject(dc, app->fontCaption);
    SetTextColor(dc, (district == game->location) ? COLOR_GOLD : COLOR_GOLD_DIM);
    SetRect(&text, cell.left + MAP_CELL_PAD, cell.top + 3, cell.right - MAP_CELL_PAD, cell.top + CAPTION_HEIGHT);
    DrawTextW(dc, MAP_NAMES[district], -1, &text, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
    SelectObject(dc, app->fontSmall);
    SetTextColor(dc, COLOR_GOLD_DIM);
    SetRect(&text, cell.left + MAP_CELL_PAD, cell.top + CAPTION_HEIGHT, cell.right - MAP_CELL_PAD, cell.top + CAPTION_HEIGHT + SMALL_FONT_HEIGHT + 2);
    DrawTextW(dc, MAP_HOODS_A[district], -1, &text, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
    SetRect(&text, cell.left + MAP_CELL_PAD, cell.top + CAPTION_HEIGHT + SMALL_FONT_HEIGHT + 2, cell.right - MAP_CELL_PAD, cell.top + CAPTION_HEIGHT + (2 * (SMALL_FONT_HEIGHT + 2)));
    DrawTextW(dc, MAP_HOODS_B[district], -1, &text, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
    for (pipIndex = 0; game->policePresence[district] > pipIndex; pipIndex++)
    {
        SelectObject(dc, app->penPolice);
        SelectObject(dc, app->brushPolice);
        Ellipse(dc, cell.left + MAP_CELL_PAD + (pipIndex * MAP_PIP_STEP), cell.bottom - MAP_PIP_SIZE - MAP_CELL_PAD, cell.left + MAP_CELL_PAD + (pipIndex * MAP_PIP_STEP) + MAP_PIP_SIZE, cell.bottom - MAP_CELL_PAD);
    }
    if (DISTRICT_LITTLE_ITALY == district)
    {
        DrawHouseIcon(app, dc, &cell);
    }
    if (DISTRICT_SUBURBS == district && 0 != game->churchFound)
    {
        DrawChurchIcon(app, dc, &cell);
    }
    DrawBizMarks(app, dc, district);
    if (0 != atWar)
    {
        SetBkMode(dc, TRANSPARENT);
        SelectObject(dc, app->fontCaption);
        SetTextColor(dc, COLOR_BLOOD);
        SetRect(&text, cell.left, cell.bottom - CAPTION_HEIGHT, cell.right - MAP_CELL_PAD - MAP_BIZ_STEP, cell.bottom - 2);
        DrawTextW(dc, L"WAR", -1, &text, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
    }
    if (PING_NONE != game->mapPing[district])
    {
        pingMark.kind = game->mapPing[district];
        pingMark.center.x = (cell.left + cell.right) / 2;
        pingMark.center.y = ((cell.top + cell.bottom) / 2) + 8;
        DrawPingMark(app, dc, &pingMark);
    }
}

static void PaintMap(App* app, HDC dc)
{
    int district = 0;

    for (district = 0; NUM_DISTRICTS > district; district++)
    {
        DrawMapCell(app, dc, district);
    }
}

static LRESULT HandlePaint(App* app)
{
    PAINTSTRUCT paint = { 0 };
    HDC dc = NULL;

    dc = BeginPaint(app->windowMain, &paint);
    if (NULL != dc)
    {
        PaintBanner(app, dc);
        PaintPane(app, dc, 0);
        PaintPane(app, dc, 1);
        PaintPane(app, dc, 2);
        PaintPane(app, dc, 3);
        PaintFamily(app, dc);
        if (0 == app->game->mapCollapsed)
        {
            PaintPane(app, dc, 4);
            PaintMap(app, dc);
        }
        EndPaint(app->windowMain, &paint);
    }

    return 0;
}

static LRESULT CALLBACK MainWndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    App* app = NULL;
    CREATESTRUCTW* createInfo = NULL;
    MEASUREITEMSTRUCT* measure = NULL;
    const DRAWITEMSTRUCT* drawItem = NULL;
    HDC controlDc = NULL;

    app = (App*)GetWindowLongPtrW(window, GWLP_USERDATA);
    switch (message)
    {
    case WM_CREATE:
        createInfo = (CREATESTRUCTW*)lParam;
        app = (App*)createInfo->lpCreateParams;
        app->windowMain = window;
        SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR)app);
        CreateControls(app);
        break;
    case WM_SIZE:
        LayoutControls(app);
        break;
    case WM_ERASEBKGND:
        PaintBackground(app, (HDC)wParam);
        result = 1;
        break;
    case WM_PAINT:
        result = HandlePaint(app);
        break;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
        controlDc = (HDC)wParam;
        SetTextColor(controlDc, COLOR_TEXT);
        SetBkColor(controlDc, COLOR_PANE);
        result = (LRESULT)app->brushPane;
        break;
    case WM_MEASUREITEM:
        measure = (MEASUREITEMSTRUCT*)lParam;
        measure->itemHeight = CHOICE_ITEM_HEIGHT;
        result = 1;
        break;
    case WM_DRAWITEM:
        drawItem = (const DRAWITEMSTRUCT*)lParam;
        if (ID_CHOICES == drawItem->CtlID)
        {
            HandleDrawList(app, drawItem);
        }
        if (ID_BUTTON == drawItem->CtlID || ID_MAPBTN == drawItem->CtlID)
        {
            HandleDrawButton(app, drawItem);
        }
        result = 1;
        break;
    case WM_COMMAND:
        if (ID_BUTTON == LOWORD(wParam))
        {
            OnChoose(app);
        }
        if (ID_MAPBTN == LOWORD(wParam))
        {
            app->game->mapCollapsed = (0 == app->game->mapCollapsed) ? 1 : 0;
            SetWindowTextW(app->windowMapButton, (0 != app->game->mapCollapsed) ? L"SHOW MAP" : L"HIDE MAP");
            LayoutControls(app);
        }
        if (ID_CHOICES == LOWORD(wParam) && LBN_DBLCLK == HIWORD(wParam))
        {
            OnChoose(app);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        result = DefWindowProcW(window, message, wParam, lParam);
        break;
    }

    return result;
}

int RunApp(void)
{
    int exitCode = 0;
    App* app = NULL;
    WNDCLASSEXW windowClass = { 0 };
    HWND window = NULL;
    MSG message = { 0 };
    BOOL gotMessage = FALSE;
    BOOL darkTitle = TRUE;
    int soundIndex = 0;

    app = (App*)AllocZeroed(sizeof(App));
    if (NULL == app)
    {
        exitCode = 1;
        goto Cleanup;
    }
    app->game = (GameState*)AllocZeroed(sizeof(GameState));
    app->formatBuffer = (WCHAR*)AllocZeroed(FORMAT_BUFFER_CHARS * sizeof(WCHAR));
    if (NULL == app->game || NULL == app->formatBuffer)
    {
        exitCode = 1;
        goto Cleanup;
    }
    SoundBuild(app);
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = MainWndProc;
    windowClass.hInstance = GetModuleHandleW(NULL);
    windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    windowClass.hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(ICON_RESOURCE_ID));
    windowClass.hIconSm = windowClass.hIcon;
    windowClass.hbrBackground = NULL;
    windowClass.lpszClassName = CLASS_NAME;
    if (0 == RegisterClassExW(&windowClass))
    {
        exitCode = 1;
        goto Cleanup;
    }
    window = CreateWindowExW(0, CLASS_NAME, WINDOW_TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, GetModuleHandleW(NULL), app);
    if (NULL == window)
    {
        exitCode = 1;
        goto Cleanup;
    }
    DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkTitle, sizeof(darkTitle));
    ShowWindow(window, SW_SHOW);
    NewGame(app);
    RefreshUi(app);
    for (;;)
    {
        gotMessage = GetMessageW(&message, NULL, 0, 0);
        if (FALSE == gotMessage || -1 == (int)gotMessage)
        {
            break;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    exitCode = (int)message.wParam;

Cleanup:
    if (NULL != app)
    {
        for (soundIndex = 0; SOUND_COUNT > soundIndex; soundIndex++)
        {
            FreeMemory(app->sounds[soundIndex]);
        }
        FreeMemory(app->formatBuffer);
        FreeMemory(app->game);
    }
    FreeMemory(app);

    return exitCode;
}
