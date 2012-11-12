/*
===================================


    NetMission Object Registry    
              Troid92              


===================================
*/


using namespace std;

class NMObjNode;
class NMObject;
class NMObjects;



void nmRegisterObject(NMObject* , char* const , long , bool , NMObjects* );
void nmDeleteObject(NMObject* , bool , NMObjects* );
NMObjNode* nmFindObject(char* const , bool , NMObjects* );
void nmSetDepth(NMObject* , bool , NMObjects* );
bool ShouldDeleteStuff();


bool DestroyingAll = false;

// ============================
//  The Universal Object Class
// ============================

// Game Maker-style objects are extremely easy to make with this class
// Just inherit it and create your own DrawEvent() and StepEvent() functions
// Be sure to use SetDepth() to change the object's depth, or it won't work


class NMObject
{
    private:
    long theDepth;
    NMObjNode* theNode;
    
    public:
    NMObject();
    ~NMObject();
    virtual void StepEvent() { }
    virtual void DrawEvent() { }
    virtual void Destroy() { delete this; }
    void Register(char* , long);
    NMObjNode* FindObject(char* );
    NMObjNode* GetNode();
    NMObjNode* SetTheNode(NMObjNode* );
    void SetDepth(long );
    long GetDepth();
};

void NMObject::Register(char* newName, long settingDepth)
{
    nmRegisterObject(this, newName, settingDepth, false, 0);
}

NMObjNode* NMObject::FindObject(char* findName)
{
    return nmFindObject(findName, false, 0);
}




NMObject::NMObject()
{
    ;
}

NMObject::~NMObject()
{
    nmDeleteObject(this, false, 0);
}

void NMObject::SetDepth(long newDepth)
{
    if (newDepth != theDepth)
    {
        theDepth = newDepth;
        nmSetDepth(this, false, 0);
    }
}

long NMObject::GetDepth()
{
    return theDepth;
}

NMObjNode* NMObject::GetNode()
{
    return theNode;
}

NMObjNode* NMObject::SetTheNode(NMObjNode* newNode)
{
    static bool firstSet = true;
    if (firstSet)
    {
        theNode = newNode;
        firstSet = false;
    }
}




// ==============
//  Object Nodes 
// ==============


class NMObjNode
{
    public:
    NMObjNode* next;
    char* name;
    NMObject* object;
    
    NMObjNode();
    //~NMObjNode() { MessageBox(NULL,"Destructor","Destructor",MB_OK); };
};

NMObjNode::NMObjNode()
{
    //MessageBox(NULL,"Constructor","Constructor",MB_OK);
    next = 0;
    object = 0;
}










// ====================
//  The Object manager
// ====================


class NMObjects
{
    private:
    NMObjNode* firstNode;
    
    public:
    NMObjects();
    void RegisterObject(NMObject* , char* const, long);
    void SetDepth(NMObject* );
    NMObjNode* FindObject(char* const);
    void Step();
    void Draw();
    bool DeleteObject(NMObject* );
    ~NMObjects();
    
};












NMObjects::NMObjects()
{
    firstNode = 0;
}

NMObjects::~NMObjects()
{
    DestroyingAll = true;
    if (firstNode != 0)
    {
        NMObjNode* currentNode = firstNode->next;
        while (currentNode != 0)
        {
            if (firstNode->object != 0) firstNode->object->Destroy();
            delete firstNode;
            firstNode = currentNode;
            currentNode = currentNode->next;
        }
        if (firstNode->object != 0) firstNode->object->Destroy();
        delete firstNode;
    }
}




void NMObjects::RegisterObject(NMObject* registerObject, char* const newName, long newDepth)
{
    //MessageBox(NULL,"Registering",newName,MB_OK);
    NMObjNode* currentNode;
    if (firstNode != 0)
    {
        currentNode = firstNode;
        bool flag = false;
        while (currentNode != 0)
        {
            if (strcmp(currentNode->name,newName) == 0 && currentNode->object == 0)
            {
                //MessageBox(NULL,"Found existing name",newName,MB_OK);
                currentNode->object = registerObject;
                currentNode->object->SetTheNode(currentNode);
                flag = true;
                break;
            }
            if (currentNode->next == 0)
            {
                //MessageBox(NULL,"Can't go any further",newName,MB_OK);
                break;
            }
            //MessageBox(NULL,"Going further",newName,MB_OK);
            currentNode = currentNode->next;
        }
        if (!flag)
        {
            //MessageBox(NULL,"Creating new name",newName,MB_OK);
            NMObjNode* newNode = new NMObjNode;
            newNode->next = currentNode->next;
            currentNode->next = newNode;
            newNode->name = newName;
            newNode->object = registerObject;
            newNode->object->SetTheNode(newNode);
            currentNode = newNode;
        }
    }
    else
    {
        //MessageBox(NULL,"Creating new name--first node",newName,MB_OK);
        firstNode = new NMObjNode;
        firstNode->name = newName;
        firstNode->object = registerObject;
        firstNode->object->SetTheNode(firstNode);
        firstNode->next = 0;
        currentNode = firstNode;
    }
    //MessageBox(NULL,"Setting the default depth of the object",newName,MB_OK);
    currentNode->object->SetDepth(newDepth);
    //MessageBox(NULL,"Depth set",newName,MB_OK);
}




NMObjNode* NMObjects::FindObject(char* const name)
{
    //MessageBox(NULL,"Finding",name,MB_OK);
    if (firstNode != 0)
    {
        NMObjNode* currentNode = firstNode;
        while (currentNode != 0)
        {
            if (strcmp(currentNode->name,name) == 0)
            {
                //MessageBox(NULL,"Found it",name,MB_OK);
                return currentNode;
            }
            if (currentNode->next == 0) break;
            currentNode = currentNode->next;
        }
        //MessageBox(NULL,"Didn't find it, reserving",name,MB_OK);
        NMObjNode* newNode = new NMObjNode;
        newNode->next = 0;
        currentNode->next = newNode;
        newNode->name = name;
        newNode->object = 0;
        return newNode;
    }
    else
    {
        //MessageBox(NULL,"Didn't find it, reserving",name,MB_OK);
        firstNode = new NMObjNode;
        firstNode->name = name;
        firstNode->object = 0;
        firstNode->next = 0;
        return firstNode;
    }
}

bool NMObjects::DeleteObject(NMObject* deleteObject)
{
    //MessageBox(NULL,"Deleting","Deleting",MB_OK);
    NMObjNode* currentNode = firstNode;
    NMObjNode* currentNode2 = firstNode;
    while (currentNode != 0)
    {
        if (currentNode->object == deleteObject)
        {
            currentNode->object = 0;
            //MessageBox(NULL,"Deleted",currentNode->name,MB_OK);
            return true;
        }
        else
        {
            currentNode2 = currentNode;
            currentNode = currentNode->next;
        }
    }
    return false;
}

// Doesn't work if a node with the object can't be found
void NMObjects::SetDepth(NMObject* theObject)
{
    //MessageBox(NULL,"Setting depth","Setting depth",MB_OK);
    NMObjNode* currentNode1 = firstNode;
    NMObjNode* currentNode2 = firstNode;
    NMObjNode* theNode = 0;
    bool success = false;
    //MessageBox(NULL,"Finding node","Finding node",MB_OK);
    while (currentNode1 != 0)
    {
        if (currentNode1->object == theObject)
        {
            if (currentNode1 == firstNode)
            {
                //MessageBox(NULL,"It's the first node",currentNode1->name,MB_OK);
                theNode = currentNode1;
                firstNode = currentNode1->next;
                theNode->next = 0;
            }
            else
            {
                //MessageBox(NULL,"Found it",currentNode1->name,MB_OK);
                theNode = currentNode1;
                currentNode2->next = currentNode1->next;
                theNode->next = 0;
            }
            success = true;
            break;
        }
        else
        {
            currentNode2 = currentNode1;
            currentNode1 = currentNode1->next;
        }
    }
    
    if (success && theNode != 0)
    {
        //MessageBox(NULL,"Moving node","Moving node",MB_OK);
        if (firstNode != 0)
        {
            bool flag = false;
            currentNode1 = firstNode;
            if (currentNode1->object->GetDepth() > theObject->GetDepth())
            {
                flag = true;
                //MessageBox(NULL,"Setting as first node",theNode->name,MB_OK);
                theNode->next = firstNode;
                firstNode = theNode;
            }
            if (!flag)
            {
                while (currentNode1->next != 0)
                {
                    if (currentNode1->next->object != 0)
                    {
                        if (currentNode1->next->object->GetDepth() > theObject->GetDepth())
                        {
                            //MessageBox(NULL,"Found position, placing in",theNode->name,MB_OK);
                            theNode->next = currentNode1->next;
                            currentNode1->next = theNode;
                            break;
                        }
                    }
                    currentNode1 = currentNode1->next;
                }
                if (currentNode1->next == 0)
                {
                    currentNode1->next = theNode;
                }
            }
        }
        else firstNode = theNode;
    }
    //else MessageBox(NULL,"Couldn't find object","Couldn't find object",MB_OK);
}


void NMObjects::Step()
{
    //MessageBox(NULL,"Stepping","Stepping",MB_OK);
    NMObjNode* currentNode = firstNode;
    while (currentNode != 0)
    {
        if (currentNode->object != 0) currentNode->object->StepEvent();
        currentNode = currentNode->next;
    }
}

void NMObjects::Draw()
{
    //MessageBox(NULL,"Drawing","Drawing",MB_OK);
    NMObjNode* currentNode = firstNode;
    while (currentNode != 0)
    {
        if (currentNode->object != 0) currentNode->object->DrawEvent();
        currentNode = currentNode->next;
    }
}





// Register NMObject to NMObjects or NMObjects to the function
void nmRegisterObject(NMObject* Object, char* const name, long depth, bool SetObjects, NMObjects* NewObjects)
{
    static NMObjects* Objects;
    if (SetObjects) Objects = NewObjects;
    else
    {
        Objects->RegisterObject(Object, name, depth);
    }
}

void nmDeleteObject(NMObject* Object, bool SetObjects, NMObjects* NewObjects)
{
    static NMObjects* Objects;
    if (SetObjects) Objects = NewObjects;
    else Objects->DeleteObject(Object);
}

NMObjNode* nmFindObject(char* const name, bool SetObjects, NMObjects* NewObjects)
{
    static NMObjects* Objects;
    if (SetObjects)
    {
        Objects = NewObjects;
        return 0;
    }
    else
    {
        return Objects->FindObject(name);
    }
}


void nmSetDepth(NMObject* Object, bool SetObjects, NMObjects* NewObjects)
{
    static NMObjects* Objects;
    if (SetObjects) Objects = NewObjects;
    else Objects->SetDepth(Object);
}


bool ShouldDeleteStuff()
{
    return !DestroyingAll;
}




// ===========================
//  The Global Vars Container
// ===========================

#define STEP nmGlobal.nmObjects.Step
#define DRAW nmGlobal.nmObjects.Draw

class NMGlobal
{
    public:
    NMGlobal();
    NMObjects nmObjects;
    ~NMGlobal();
    
} nmGlobal;

NMGlobal::NMGlobal()
{
    nmRegisterObject(0,0,0,true,&nmObjects);
    nmDeleteObject(0,true,&nmObjects);
    nmFindObject(0,true,&nmObjects);
    nmSetDepth(0,true,&nmObjects);
}

NMGlobal::~NMGlobal()
{
    ;
}


