#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "xalloc.h"

struct node {
    /// flags of this node
    uint32_t flags;
    /// 256 bit mask
    uint16_t grp[16];
    /// left and right node of this binary tree
    uint32_t left, right;
};

struct engine {
    /// all nodes within the engine
    struct node *nodes;
    /// the number of nodes
    uint32_t num_nodes;
    /// the number of allocated nodes
    uint32_t a_nodes;
};

struct node *add_node(struct engine *eng)
{
    struct node *n;

    if (eng->num_nodes + 1 >= eng->a_nodes) {
        eng->a_nodes *= 2;
        eng->a_nodes++;
        eng->nodes = xreallocarray(eng->nodes, sizeof(*eng->nodes),
                                   eng->a_nodes);
    }
    n = &eng->nodes[eng->num_nodes++];
    n->flags = 0;
    n->left = UINT32_MAX;
    n->right = UINT32_MAX;
    memset(n->grp, 0, sizeof(n->grp));
    return n;
}

void gr_toggle(uint16_t *grp, unsigned char ch)
{
    grp[ch >> 4] |= 1 << (ch & 0xf);
}

bool gr_is_toggled(uint16_t *grp, unsigned char ch)
{
    return grp[ch >> 4] & (1 << (ch & 0xf));
}

int parse_regex(const char *regex, struct engine *eng)
{
    struct node     *new_head;
    size_t          *jp = NULL;
    size_t          num_jp = 0;
    struct node     *root;

    root = NULL;
    for (; regex[0] != '\0'; ) {
        switch (regex[0]) {
        case '(':
            jp = xrealloc(jp, sizeof(*jp) * (num_jp + 1));
            jp[num_jp] = eng->num_nodes;
            num_jp++;
            regex++;
            break;
            
        case '[':
            fprintf(stderr, "'[' is not supported yet\n");
            abort();
            break;

        case ')':
            if (num_jp != 0) {
                regex++;
                switch (regex[0]) {
                case '+':
                    if (eng->nodes[eng->num_nodes - 1].right == SIZE_MAX) {
                        eng->nodes[eng->num_nodes - 1].right = jp[num_jp - 1];
                    } else {
                        /* TODO: create intermezzo node */
                        fprintf(stderr, "hit edge case #1\n");
                        abort();
                    }
                    regex++;
                    break;
                
                case '*':
                case '?':
                    fprintf(stderr, "*? not supported yet\n");
                    abort();
                    break;
                }
                /* TODO: */
                break;
            }
            /* fall through */
        default:
            new_head = add_node(eng);
            gr_toggle(new_head->grp, regex[0]);
            if (eng->num_nodes > 1) {
                eng->nodes[eng->num_nodes - 2].left = new_head - eng->nodes;
            }
            regex++;
            switch (regex[0]) {
            case '+':
                /* repeat to itself */
                eng->nodes[eng->num_nodes - 1].right = eng->num_nodes - 1;
                regex++;
                break;

            case '*':
            case '?':
                fprintf(stderr, "*? not supported yet\n");
                abort();
                break;
            }
        }
    }
    free(jp);
    return 0;
}

void print_group(uint16_t *grp)
{
    int         i;

    for (i = 0; i < 256; i++) {
        if (i % 16 == 0) {
            printf("\n");
        }
        if (gr_is_toggled(grp, i)) {
            printf("1");
        } else {
            printf("0");
        }
        //printf("%c", node->grp[i >> 4] & 0xf);
    }
}

void print_node(struct engine *eng, uint32_t index)
{
    struct node *n;

    if (index == UINT32_MAX) {
        printf("(empty node)\n");
        return;
    }
    n = &eng->nodes[index];
    print_group(n->grp);
    printf("\n");
    printf("left: ");
    print_node(eng, n->left);
    printf("right: ");
    print_node(eng, n->right);
}

int match_string(struct engine *eng, uint32_t index, const char *s)
{
    struct node     *n;
    unsigned char   c;
    int             l, r;

    if (index == UINT32_MAX || s[0] == '\0') {
        return 0;
    }
    n = &eng->nodes[index];
    c = s[0];
    if (gr_is_toggled(n->grp, c)) {
        l = match_string(eng, n->left, &s[1]);
        r = match_string(eng, n->right, &s[1]);
        if (l < r) {
            return r + 1;
        }
        return l + 1;
    }
    printf("failed here: %s\n", s);
    return -1;
}

int main(void)
{
    const char *regex = "a+b+c";
    struct engine eng;
    struct node *n;
    
    if (parse_regex(regex, &eng) != 0) {
        printf("regex '%s' is invalid\n", regex);
        return 1;
    }
    //printf("head: ");
    //print_node(&eng, 0);
    printf("got %u nodes\n", eng.num_nodes);
    for (uint32_t i = 0; i < eng.num_nodes; i++) {
        n = &eng.nodes[i];
        print_group(n->grp);
        printf("\n");
    }
    printf("match: %d\n", match_string(&eng, 0, "aaabbbccc"));
    free(eng.nodes);
    return 0;
}
