//------------  ------------------------------------------------------------------
/*
    This file is part of sdchaind: https://github.com/SDChain/SDChain-core
    Copyright (c) 2012, 2015 SDChain Alliance.



*/
//==============================================================================

#include <BeastConfig.h>
#include <sdchain/ledger/BookDirs.h>
#include <sdchain/ledger/View.h>
#include <sdchain/protocol/Indexes.h>

namespace sdchain {

BookDirs::BookDirs(ReadView const& view, Book const& book)
    : view_(&view)
    , root_(keylet::page(getBookBase(book)).key)
    , next_quality_(getQualityNext(root_))
    , key_(view_->succ(root_, next_quality_).value_or(zero))
{
    assert(root_ != zero);
    if (key_ != zero)
    {
        if (! cdirFirst(*view_, key_, sle_, entry_, index_,
            beast::Journal()))
        {
            assert(false);            
        }
    }
}

auto
BookDirs::begin() const ->
    BookDirs::const_iterator
{
    auto it = BookDirs::const_iterator(*view_, root_, key_);
    if (key_ != zero)
    {
        it.next_quality_ = next_quality_;
        it.sle_ = sle_;
        it.entry_ = entry_;
        it.index_ = index_;
    }
    return it;
}

auto
BookDirs::end() const  ->
    BookDirs::const_iterator
{
    return BookDirs::const_iterator(*view_, root_, key_);
}


beast::Journal BookDirs::const_iterator::j_ = beast::Journal();

bool
BookDirs::const_iterator::operator==
    (BookDirs::const_iterator const& other) const
{
    if (view_ == nullptr || other.view_ == nullptr)
        return false;

    assert(view_ == other.view_ && root_ == other.root_);
    return entry_ == other.entry_ &&
        cur_key_ == other.cur_key_ &&
            index_ == other.index_;
}

BookDirs::const_iterator::reference
BookDirs::const_iterator::operator*() const
{
    assert(index_ != zero);
    if (! cache_)
        cache_ = view_->read(keylet::offer(index_));
    return *cache_;
}

BookDirs::const_iterator&
BookDirs::const_iterator::operator++()
{
    assert(index_ != zero);
    if (! cdirNext(*view_, cur_key_, sle_, entry_, index_, j_))
    {
        if (index_ != 0 ||
            (cur_key_ = view_->succ(++cur_key_,
                next_quality_).value_or(zero)) == zero)
        {
            cur_key_ = key_;
            entry_ = 0;
            index_ = zero;
        }
        else if (! cdirFirst(*view_, cur_key_,
            sle_, entry_, index_, j_))
        {
            assert(false);
        }
    }

    cache_ = boost::none;
    return *this;
}

BookDirs::const_iterator
BookDirs::const_iterator::operator++(int)
{
    assert(index_ != zero);
    const_iterator tmp(*this);
    ++(*this);
    return tmp;
}

} // sdchain
