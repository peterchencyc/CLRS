/***************************************************************************
 *  @file       redblacktree.hpp
 *  @author     Alan.W
 *  @date       05  July 2014
 *  @remark     Algorithims implementation for CLRS, using C++ templates.
 ***************************************************************************/
#ifndef REDBLACKTREE_HPP
#define REDBLACKTREE_HPP
#include <memory>
#include <functional>
#include "node.hpp"


namespace ch13{

/**
 * @brief The RedBlackTree class
 */
template<typename K, typename D>
class RedBlackTree
{
public:
    using KeyType   =   K;
    using DataType  =   D;
    using NodeType  =   Node<K,D>;
    using sPointer  =   std::shared_ptr<NodeType>;
    using wPointer  =   std::weak_ptr<NodeType>;

    RedBlackTree():
        nil(std::make_shared<NodeType>())
    {
        root = nil;
    }

    void remove(sPointer target)
    {
        sPointer x,y;
        Color  y_original_color;

        if(target->left == this->nil)
        {
            y = target;
            y_original_color = y->color;
            x = y->right;
            transplant(target,x);
        }
        else if(target->right   ==  this->nil)
        {
            y = target;
            y_original_color = y->color;
            x = y->left;
            transplant(target,x);
        }
        else
        {
            y = minimum(target->right);
            y_original_color = y->color;
            x = y->right;

            if(y->parent.lock() ==  target)
                x->parent   =   y;
            else
            {
                transplant(y,x);
                y->right    =   target->right;
                y->right->parent    =   y;
            }

            transplant(target, y);
            y->left         =   target->left;
            y->left->parent =   y;
            y->color        =   target->color;
        }

        if(y_original_color ==  Color::BLACK)
            //  to do   : add fix up function
        {}
    }

    /**
     * @brief print
     *
     * O(n)
     */
    void print()const
    {
        std::cout << "-----------------------------\n";
        std::cout << debug::yellow("root= ") << root
                  << " key = " << root->key    << std::endl;

        std::cout << debug::yellow("nil = ") << nil
                  << std::endl;

        std::function<void(sPointer node)> inorder_walk =
                [&inorder_walk,this](sPointer node)
        {
            if(node != nil)
            {
                inorder_walk(node->left);
                node->print();
                inorder_walk(node->right);
            }
        };
        inorder_walk(root);
    }

    /**
     * @brief insert
     * @param key
     *
     * O(h)
     */
    void insert(const KeyType& key)
    {
        sPointer added = std::make_shared<NodeType>(key);
        insert(added);
    }

    /**
     * @brief insert
     * @param added
     *
     * @page    315
     * O(h)
     */
    void insert(sPointer added)
    {
        sPointer tracker = nil;
        sPointer curr = root;
        while(curr != nil)
        {
            tracker =   curr;
            curr    =   curr->key > added->key?     curr->left  :   curr->right;
        }

        added->parent   =   tracker;
        if(tracker == nil)
            root    =   added;
        else
            (added->key < tracker->key?     tracker->left   :   tracker->right)
                    =   added;

        added->left =   added->right    =   nil;
        added->color=   Color::RED;

        insert_fixup(added);
    }

    /**
     * @brief search
     * @param key
     *
     * @complx  O(h)
     */
    sPointer search(const KeyType& key)const
    {
        sPointer ret  = nullptr;
        sPointer curr = this->root;
        while(curr)
        {
            if(curr->key    ==  key)
            {
                ret = curr;
                break;
            }
            else
                curr = (curr->key > key?    curr->left  :   curr->right);
        }
        return ret;
    }

    /**
     * @brief left_rotate
     * @param x
     *
     *      [x]                    [y]
     *         \        =>        /
     *          [y]            [x]
     *
     * @complx  o(1)
     *
     * @page    313
     */
    void left_rotate(sPointer x)
    {
        sPointer y = x->right;      //  set y

        x->right    =   y->left;    //  turn y's left into x's right
        if(y->left  !=  this->nil)
            y->left->parent = x;

        y->parent   =   x->parent;  //  link x's parent to y
        if(x->parent.lock()    ==  this->nil)
            this->root  =   y;
        else
            (x->is_left()?  x->parent.lock()->left  :  x->parent.lock()->right)
                        =   y;

        y->left     =   x;
        x->parent   =   y;
    }

    /**
     * @brief right_rotate
     * @param y
     *
     *          [y]             [x]
     *          /       =>         \
     *        [x]                   [y]
     *
     * @complx  O(1)
     */
    void right_rotate(sPointer y)
    {
        sPointer x = y->left;

        y->left = x->right;
        if(x->right != this->nil)
            x->right->parent    =   y;

        x->parent   =   y->parent;
        if(y->parent.lock() ==  this->nil)
            root    =   x;
        else
            (y->is_left()?  y->parent.lock()->left  :  y->parent.lock()->right)
                    =   x;

        x->right    =   y;
        y->parent   =   x;
    }
//private:
    sPointer root;
    sPointer nil;

    /**
     * @brief transplant
     * @param to
     * @param from
     *
     * @note    a bit different from the BST version
     * @complx  O(1)
     * @page    323
     */
    void transplant(sPointer to, sPointer from)    
    {
        if(ascend(to,1) == this->nil)
            this->root  =   from;
        else
            (to->is_left()?     ascend(to,1)->left    :   ascend(to,1)->right)
                        =   from;

        from->parent    =   to->parent;
    }

    /**
     * @brief minimum
     */
    sPointer minimum(sPointer node)
    {
        assert(node != this->nil);

        sPointer tracker = node;

        while(node != this->nil)
        {
            tracker =   node;
            node    =   node->left;
        }
        return tracker;
    }

    /**
     * @brief ascend
     *
     * used for insert_fixup
     */
    sPointer ascend(sPointer node, int level)
    {
        while(level--)
            node = node->parent.lock();
        return node;
    }

    /**
     * @brief insert_fixup
     *
     * @complx  O(1)
     *
     * based on the pyseudocode on Page 316
     */
    void insert_fixup(sPointer added)
    {
        while(ascend(added,1)->color   ==  Color::RED)
        {
            if(ascend(added,1)->is_left())
            {
                sPointer uncle = ascend(added,2)->right;
                if(uncle->color ==  Color::RED)
                {
                    uncle->color            =   Color::BLACK;
                    ascend(added,1)->color  =   Color::BLACK;
                    ascend(added,2)->color  =   Color::RED;
                    added   =   ascend(added,2);
                }
                else
                {
                    if(added->is_right())
                    {
                        added   =   ascend(added,1);
                        left_rotate(added);
                    }
                    ascend(added,1)->color  =   Color::BLACK;
                    ascend(added,2)->color  =   Color::RED;
                    right_rotate(ascend(added,2));
                }
            }
            else
            {
                sPointer uncle = ascend(added,2)->left;
                if(uncle->color ==  Color::RED)
                {
                    uncle->color            =   Color::BLACK;
                    ascend(added,1)->color  =   Color::BLACK;
                    ascend(added,2)->color  =   Color::RED;
                    added   =   ascend(added,2);
                }
                else
                {
                    if(added->is_left())
                    {
                        added   =   ascend(added,1);
                        right_rotate(added);
                    }
                    ascend(added,1)->color  =   Color::BLACK;
                    ascend(added,2)->color  =   Color::RED;
                    left_rotate(ascend(added,2));
                }
            }
        }
        root->color = Color::BLACK;
    }
};
}//namespace


#endif // REDBLACKTREE_HPP

//! testing insert()
//#include <iostream>
//#include <vector>
//#include <node.hpp>
//#include "redblacktree.hpp"

//int main()
//{
//    std::vector<int> v =
//        {6,5,1,1,2,34,5,6,7,8,9,8,7,4,3,2,2,3,5,6,7,2,345};
//    ch13::RedBlackTree<int,std::string> tree;

//    for(auto i : v)
//        tree.insert(i);

//    tree.print();
//    std::cout << "length = " << v.size() << std::endl;

//    return 0;
//}