#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "xalloc.h"

struct node {
    /// 256 bit mask
    uint16_t grp[16];
    /// left and right node of this binary tree
    struct node *left, *right;
};

void gr_toggle(uint16_t *grp, unsigned char ch)
{
    grp[ch >> 4] |= 1 << (ch & 0xf);
}

bool gr_is_toggled(uint16_t *grp, unsigned char ch)
{
    return grp[ch >> 4] & (1 << (ch & 0xf));
}

struct node *parse_regex(const char *regex)
{
    struct node     *new_head;
    struct node     **nodes = NULL;
    size_t          num_nodes = 0;
    size_t          *jp = NULL;
    size_t          num_jp = 0;
    struct node     *root;

    root = NULL;
    for (; regex[0] != '\0'; ) {
        switch (regex[0]) {
        case '(':
            jp = xrealloc(jp, sizeof(*jp) * (num_jp + 1));
            jp[num_jp] = num_nodes;
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
                    if (nodes[num_nodes - 1]->right == NULL) {
                        nodes[num_nodes - 1]->right =
                            nodes[jp[num_jp - 1]];
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
                num_nodes = jp[--num_jp];
                break;
            }
            /* fall through */
        default:
            nodes = xrealloc(nodes, sizeof(*nodes) * (num_nodes + 1));
            new_head = xmalloc(sizeof(*new_head));
            nodes[num_nodes++] = new_head;
            new_head->left = NULL;
            new_head->right = NULL;
            memset(new_head->grp, 0, sizeof(new_head->grp));
            gr_toggle(new_head->grp, regex[0]);
            if (num_nodes > 1) {
                nodes[num_nodes - 2]->left = new_head;
            }
            regex++;
            switch (regex[0]) {
            case '+':
                /* repeat to itself */
                nodes[num_nodes - 1]->right = nodes[num_nodes - 1];
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
    root = nodes[0];
    free(nodes);
    return root;
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

void print_node(struct node *node)
{
    if (node == NULL) {
        printf("(empty node)\n");
        return;
    }
    print_group(node->grp);
    printf("\n");
    printf("left: ");
    print_node(node->left);
    printf("right: ");
    print_node(node->right);
}

void free_node(struct node *node)
{
    free(node->left);
    free(node->right);
    free(node);
}

int match_string(struct node *n, const char *s)
{
    unsigned char       c;
    int                 l, r;

    if (s[0] == '\0') {
        return 0;
    }
    c = s[0];
    if (gr_is_toggled(n->grp, c)) {
        l = 0;
        r = 0;
        if (n->left != NULL) {
            l = match_string(n->left, &s[1]);
        }
        if (n->right != NULL) {
            r = match_string(n->right, &s[1]);
        }
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
    const char *regex = "a+b";
    
    struct node *n = parse_regex(regex);
    //printf("head: ");
   // print_node(n);
    printf("match: %d\n", match_string(n, "aaabc"));
    return 0;
}
