#pragma once

//////////////////////////////////////////////////////////////////////////

class DocumentImpl
{
public:
    DocumentImpl();
    DocumentImpl(int) {}
    virtual void Exclamation(std::string) {}

    virtual void SetModified(bool=true) {}
    virtual bool GetModified() const
    {
        return 0;
    }

    virtual void ShowHint(std::string) {}
    virtual void HideHint() {}

    virtual void Nag() {}
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CMainDocument : public utils::singleton<CMainDocument>
{
public:

    CMainDocument()
        : pimpl(0)
    {
    }

    void SetDocument(DocumentImpl *p)
    {
        pimpl=p;
    }

    DocumentImpl *GetDocument()
    {
        static DocumentImpl  dummy_(0);
        return pimpl ? pimpl : &dummy_;
    }

private:
    DocumentImpl *pimpl;
};

inline DocumentImpl &MainDocument()
{
    return *CMainDocument::instance()->GetDocument();
};


//////////////////////////////////////////////////////////////////////////

inline DocumentImpl::DocumentImpl()
{
    CMainDocument::instance()->SetDocument(this);
}

//

inline DocumentImpl &Document()
{
    return *CMainDocument::instance()->GetDocument();
}
