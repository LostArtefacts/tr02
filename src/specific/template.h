#ifndef TEMPLATE_H
#define TEMPLATE_H

struct STRING_FLAGGED {
    LPTSTR lpString;
    bool isPresented;
    STRING_FLAGGED(DWORD size = 256)
    {
        lpString = new char[size];
        *lpString = 0;
        isPresented = true;
    }
    ~STRING_FLAGGED();
    operator LPTSTR()
    {
        return lpString;
    }
    STRING_FLAGGED &operator=(const STRING_FLAGGED &src)
    {
        if (this != &src) {
            if (!isPresented)
                lpString = new char[strlen(src.lpString) + 1];
            strcpy(lpString, src.lpString);
        }
        return *this;
    }
};

inline STRING_FLAGGED::~STRING_FLAGGED()
{
    if (isPresented)
        delete[] lpString;
}

template <class T> struct LIST {
    struct NODE {
        NODE *next;
        NODE *previous;
        T body;
        NODE(NODE *prevNode, NODE *nextNode) : next(nextNode), previous(prevNode)
        {
        }
    };
    NODE *head;
    NODE *tail;
    DWORD dwCount;
    LIST()
    {
        head = NULL;
        tail = NULL;
        dwCount = 0;
    }
    ~LIST()
    {
        RemoveAll();
    }
    void RemoveAll()
    {
        NODE *node;
        NODE *nextNode;

        for (node = head; node; node = nextNode) {
            nextNode = node->next;
            delete node;
        }
        head = NULL;
        tail = NULL;
        dwCount = 0;
    }
    T &NewHead()
    {
        NODE *node = new NODE(NULL, head);

        if (head)
            head->previous = node;
        else
            tail = node;
        head = node;
        dwCount++;
        return node->body;
    }
    T &NewTail()
    {
        NODE *node = new NODE(tail, NULL);

        if (tail)
            tail->next = node;
        else
            head = node;
        tail = node;
        dwCount++;
        return node->body;
    }
    void AddHead(T &item)
    {
        NewHead() = item;
    }
    void AddTail(T &item)
    {
        NewTail() = item;
    }
    NODE *GetHead()
    {
        return head;
    }
    NODE *GetNext(NODE *node)
    {
        if (node)
            node = node->next;
        return node;
    }
    T &operator[](NODE *node)
    {
        return node->body;
    }
    DWORD GetCount()
    {
        return dwCount;
    }
    void Insert(T &item)
    {
        NODE *node;
        NODE *newNode;
        NODE *prev;

        if (!head) {
            AddHead(item);
            return;
        }
        for (node = head; node; node = node->next) {
            if (item < node->body) {
                prev = node->previous;
                newNode = new NODE(prev, node);
                newNode->body = item;
                if (!prev)
                    head = newNode;
                else
                    prev->next = newNode;
                node->previous = newNode;
                return;
            }
        }
        AddTail(item);
    }
    LIST &operator=(LIST &src)
    {
        NODE *node;

        if (this != &src) {
            RemoveAll();
            for (node = src.head; node; node = node->next)
                NewTail() = node->body;
        }
        return *this;
    }
};

struct GENERIC_ADAPTER {
    LPGUID lpAdapterGuid;
    GUID adapterGuid;
    STRING_FLAGGED description;
    STRING_FLAGGED name;
    void SetGuid(const GUID *lpGuid)
    {
        if (lpGuid) {
            adapterGuid = *lpGuid;
            lpAdapterGuid = &adapterGuid;
        } else {
            memset(&adapterGuid, 0, sizeof(GUID));
            lpAdapterGuid = NULL;
        }
    }
};

#endif
