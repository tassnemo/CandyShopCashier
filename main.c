#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define CANDY_KINDS 6U
#define BASKET_MAX 8U
#define NAME_LEN 16U

typedef struct {
    char name[NAME_LEN];
    uint16_t price;
    uint16_t stock;
    uint16_t sold;
} Candy_t;

typedef struct {
    uint8_t candyId;
    uint8_t qty;
} Line_t;

static Candy_t shelf[CANDY_KINDS];
static Line_t basket[BASKET_MAX];
static uint8_t basketLines;
static uint32_t cashDrawer;

static int readUInt(const char *prompt, uint32_t *value)
{
    unsigned int input;
    int character;

    printf("%s", prompt);
    if (scanf("%u", &input) != 1) {
        while ((character = getchar()) != '\n' && character != EOF) {
        }
        printf("Invalid input.\n");
        return 0;
    }
    *value = (uint32_t)input;
    while ((character = getchar()) != '\n' && character != EOF) {
    }
    return 1;
}

static int basketLineFor(uint8_t candyId)
{
    uint8_t line;

    for (line = 0U; line < basketLines; ++line) {
        if (basket[line].candyId == candyId) {
            return (int)line;
        }
    }
    return -1;
}

static uint8_t basketQuantity(uint8_t candyId)
{
    int line = basketLineFor(candyId);

    return line < 0 ? 0U : basket[line].qty;
}

static void openShop(void)
{
    const char *names[CANDY_KINDS] = {
        "Lollipop", "Gummy Bears", "Chocolate", "Caramel",
        "Jelly Beans", "Toffee"
    };
    uint16_t prices[CANDY_KINDS] = {25U, 40U, 75U, 50U, 30U, 60U};
    uint16_t quantities[CANDY_KINDS] = {20U, 15U, 12U, 10U, 18U, 8U};
    uint8_t candy;

    for (candy = 0U; candy < CANDY_KINDS; ++candy) {
        strcpy(shelf[candy].name, names[candy]);
        shelf[candy].price = prices[candy];
        shelf[candy].stock = quantities[candy];
        shelf[candy].sold = 0U;
    }
    basketLines = 0U;
    cashDrawer = 0U;
}

static void showShelf(void)
{
    uint8_t candy;

    printf("\nNo. %-15s Price  Stock\n", "Candy");
    for (candy = 0U; candy < CANDY_KINDS; ++candy) {
        printf("%2u. %-15s %5u  ", (unsigned)(candy + 1U),
               shelf[candy].name, (unsigned)shelf[candy].price);
        if (shelf[candy].stock == 0U) {
            printf("SOLD OUT\n");
        } else {
            printf("%5u\n", (unsigned)shelf[candy].stock);
        }
    }
}

static void addToBasket(void)
{
    uint32_t candyInput;
    uint32_t quantityInput;
    uint8_t candyId;
    uint8_t quantity;
    int line;
    uint8_t available;

    if (!readUInt("Candy number: ", &candyInput) ||
        !readUInt("Quantity: ", &quantityInput)) {
        return;
    }
    if (candyInput == 0U || candyInput > CANDY_KINDS ||
        quantityInput == 0U || quantityInput > 255U) {
        printf("That candy or quantity is not valid.\n");
        return;
    }
    candyId = (uint8_t)(candyInput - 1U);
    quantity = (uint8_t)quantityInput;
    available = (uint8_t)(shelf[candyId].stock - basketQuantity(candyId));
    if (quantity > available) {
        printf("Not enough stock.\n");
        return;
    }
    line = basketLineFor(candyId);
    if (line >= 0) {
        basket[line].qty = (uint8_t)(basket[line].qty + quantity);
    } else if (basketLines < BASKET_MAX) {
        basket[basketLines].candyId = candyId;
        basket[basketLines].qty = quantity;
        ++basketLines;
    } else {
        printf("The basket is full.\n");
        return;
    }
    printf("Added to basket.\n");
}

static void removeFromBasket(void)
{
    uint32_t lineInput;
    uint8_t line;

    if (basketLines == 0U) {
        printf("The basket is empty.\n");
        return;
    }
    if (!readUInt("Basket line to remove: ", &lineInput) ||
        lineInput == 0U || lineInput > basketLines) {
        printf("That basket line is not valid.\n");
        return;
    }
    line = (uint8_t)(lineInput - 1U);
    while ((uint8_t)(line + 1U) < basketLines) {
        basket[line] = basket[(uint8_t)(line + 1U)];
        ++line;
    }
    --basketLines;
    printf("Line removed.\n");
}

static uint32_t basketTotal(void)
{
    uint32_t total = 0U;
    uint8_t line;

    for (line = 0U; line < basketLines; ++line) {
        total += (uint32_t)shelf[basket[line].candyId].price * basket[line].qty;
    }
    return total;
}

static void showBasket(void)
{
    uint8_t line;

    printf("\nLine Candy             Qty Each  Cost\n");
    for (line = 0U; line < basketLines; ++line) {
        uint8_t candy = basket[line].candyId;
        uint32_t cost = (uint32_t)shelf[candy].price * basket[line].qty;

        printf("%4u %-16s %3u %4u %5u\n", (unsigned)(line + 1U),
               shelf[candy].name, (unsigned)basket[line].qty,
               (unsigned)shelf[candy].price, (unsigned)cost);
    }
    printf("Total: %u piastres\n", (unsigned)basketTotal());
}

static void giveChange(uint32_t change)
{
    const uint16_t coins[5] = {500U, 200U, 100U, 50U, 25U};
    uint32_t counts[5] = {0U, 0U, 0U, 0U, 0U};
    uint8_t coin;

    if (change == 0U) {
        printf("No change, thank you.\n");
        return;
    }
    for (coin = 0U; coin < 5U; ++coin) {
        counts[coin] = (uint8_t)(change / coins[coin]);
        change %= coins[coin];
    }
    printf("Change: 500s=%u, 200s=%u, 100s=%u, 50s=%u, 25s=%u",
            (unsigned)counts[0], (unsigned)counts[1], (unsigned)counts[2],
            (unsigned)counts[3], (unsigned)counts[4]);
    if (change != 0U) {
        printf("; %u piastres cannot be represented", (unsigned)change);
    }
    printf(".\n");
}

static void checkout(void)
{
    uint32_t total = basketTotal();
    uint32_t payment;
    uint8_t line;

    if (basketLines == 0U) {
        printf("The basket is empty.\n");
        return;
    }
    printf("Total due: %u piastres\n", (unsigned)total);
    if (!readUInt("Money received: ", &payment)) {
        return;
    }
    if (payment < total) {
        printf("That is not enough money.\n");
        return;
    }
    for (line = 0U; line < basketLines; ++line) {
        Candy_t *candy = &shelf[basket[line].candyId];

        candy->stock = (uint16_t)(candy->stock - basket[line].qty);
        candy->sold = (uint16_t)(candy->sold + basket[line].qty);
    }
    cashDrawer += total;
    giveChange(payment - total);
    basketLines = 0U;
    printf("Sale complete.\n");
}

static uint8_t bestSeller(void)
{
    uint8_t best = 0U;
    uint8_t candy;

    for (candy = 1U; candy < CANDY_KINDS; ++candy) {
        if (shelf[candy].sold > shelf[best].sold) {
            best = candy;
        }
    }
    return best;
}

static void dayReport(void)
{
    uint16_t totalSold = 0U;
    uint8_t candy;
    uint8_t best;

    best = bestSeller();
    for (candy = 0U; candy < CANDY_KINDS; ++candy) {
        totalSold = (uint16_t)(totalSold + shelf[candy].sold);
    }
    printf("\nDay report\nMoney in drawer: %u piastres\nCandies sold: %u\n",
           (unsigned)cashDrawer, (unsigned)totalSold);
    printf("Best seller: %s\nSold out: ", shelf[best].name);
    for (candy = 0U; candy < CANDY_KINDS; ++candy) {
        if (shelf[candy].stock == 0U) {
            printf("%s ", shelf[candy].name);
        }
    }
    printf("\n");
}

int main(void)
{
    uint32_t choice;

    openShop();
    do {
        printf("\n1 Shelf  2 Add  3 Remove  4 Basket  5 Checkout  6 Report  0 Exit\n");
        if (!readUInt("Choose: ", &choice)) {
            continue;
        }
        switch (choice) {
        case 1U: showShelf(); break;
        case 2U: addToBasket(); break;
        case 3U: removeFromBasket(); break;
        case 4U: showBasket(); break;
        case 5U: checkout(); break;
        case 6U: dayReport(); break;
        case 0U: printf("Goodbye.\n"); break;
        default: printf("Unknown choice.\n"); break;
        }
    } while (choice != 0U);
    return 0;
}