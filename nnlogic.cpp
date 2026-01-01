#include <cstdlib>
#include <iterator>
#include "utils/tarslog.h"
#include "common/nnlogic.h"

namespace nnlogic
{
    const short CONST_CARD_NUM = 52;

    int nnrand(int max, int min)
    {
        std::random_device rd;
        srand(rd());
        return min + rand() % (max - min + 1);
    }

    void build(vecc_t &vSrcCards)
    {
        if (!vSrcCards.empty())
        {
            vSrcCards.clear();
        }

        //
        vSrcCards.reserve(CONST_CARD_NUM);
        //8,9,10,J,Q,K,A
        for (int j = 8; j <= 14; j++)
        {
            // vSrcCards.push_back(E_NN_CARD::NN_CARD_DIAMOND + j);
            // vSrcCards.push_back(E_NN_CARD::NN_CARD_CLUB + j);
            // vSrcCards.push_back(E_NN_CARD::NN_CARD_HEART + j);
            // vSrcCards.push_back(E_NN_CARD::NN_CARD_SPADE + j);
        }
    }

    void deal(vecc_t &vSrcCards, vecc_t &vDstCards, int iNum)
    {
        for (int i = 0; i < iNum; i++)
        {
            vDstCards.push_back(vSrcCards.back());
            vSrcCards.pop_back();
        }
    }

    int getbombnum(const std::map<card_t, short> &mCardCount)
    {
        int max = 0;

        for (auto it = mCardCount.begin(); it != mCardCount.end(); it++)
        {
            if (it->second > max)
            {
                max = it->second;
            }
        }

        return max;
    }

    bool issamekind(const vecc_t &vSrcCards)
    {
        for (auto it = vSrcCards.begin() + 1; it != vSrcCards.end(); it++)
        {
            if (nncard::getNNType(*it) != nncard::getNNType(*vSrcCards.begin()))
            {
                return false;
            }
        }

        return true;
    }

    bool isstraight(const vecc_t &vNumCards, const std::map<card_t, short> &mCardCount, int space)
    {
        for (auto it = mCardCount.begin();
                it != mCardCount.end();
                it++)
        {
            if (it->second > 1)
            {
                return false;
            }
        }

        vecc_t vTestCards;
        for (int i = 2; i <= space + 1; i++)
        {
            vTestCards.push_back(i);
        }

        vTestCards.push_back(14);
        if (vecfind<card_t>(vTestCards, vNumCards))
        {
            return true;
        }

        for (auto it1 = vNumCards.begin(); it1 != vNumCards.end() - 1; it1++)
        {
            for (auto it2 = it1 + 1; it2 != vNumCards.end(); it2++)
            {
                if (abs(*it1 - *it2) > space)
                {
                    return false;
                }
            }
        }

        return true;
    }

    void sortCard(vecc_t &vTempCarsd)
    {
        std::sort(vTempCarsd.begin(), vTempCarsd.end(), [](card_t c1, card_t c2)->bool
        {
            if(nncard::getNNNum(c1) != nncard::getNNNum(c2))
            {
                return nncard::getNNNum(c1) < nncard::getNNNum(c2);
            }
            else
            {
                return nncard::getNNType(c1) < nncard::getNNType(c2);
            }
        });

        // DLOG_TRACE(" >>>>>>>>>>>>>>>>>>>>>> ");
        // for(auto card: vTempCarsd)
        // {
        //     DLOG_TRACE(" compare vTempCarsd num: "<<card <<", face: "<< nncard::getNNNum(card)<< ", suit: "<< nncard::getNNType(card));
        // }
        // DLOG_TRACE(" <<<<<<<<<<<<<<<<<<<<<<<<< ");
    }

    card_t BestCardStraight(std::map<card_t, short>& mCardCount, const vecc_t &vHallCards, int suit)
    {
        if(mCardCount.rbegin()->first - mCardCount.begin()->first > 4)
        {
            return -1;
        }
        int face = mCardCount.rbegin()->first;
        int changface = face;
        for(auto rit = mCardCount.rbegin(); rit != mCardCount.rend(); rit++)
        {
            if(rit == mCardCount.rbegin())
            {
                continue;
            }

            DLOG_TRACE(" BestCardStraight changface : "<<changface <<", face: "<< rit->first);
            if(changface - rit->first != 1)
            {
                auto it = std::find_if(vHallCards.begin(), vHallCards.end(), [changface, suit](card_t card)->bool{
                    return  getNNNum(card) == changface - 1 && (suit == -1 ||  getNNType(card) == suit);
                });
                if(it != vHallCards.end())
                {
                    return *it; 
                }  
            }
            else
            {
                changface = rit->first;
            }
        }
        if(face != 14)
        {
            auto it = std::find_if(vHallCards.begin(), vHallCards.end(), [face, suit](card_t card)->bool{
                return  getNNNum(card) == face + 1 && (suit == -1 ||  getNNType(card) == suit);
            });
            if(it != vHallCards.end())
            {
                return  *it;
            }
        }
        else
        {
            auto it = std::find_if(vHallCards.begin(), vHallCards.end(), [face, suit](card_t card)->bool{
                return  getNNNum(card) == face - 4 && (suit == -1 ||  getNNType(card) == suit);
            });
            if(it != vHallCards.end())
            {
                return *it;
            }
        }
        return -1;
    }

    void BestCards(const vecc_t &vSrcCards, const vecc_t &vWallCards, vecc_t &vDesCards)
    {
        vDesCards.insert(vDesCards.begin(), vSrcCards.begin(),vSrcCards.end());
        vecc_t vTempCarsd;
        vTempCarsd.insert(vTempCarsd.begin(), vSrcCards.begin(), vSrcCards.end());
        vecc_t vHallCards;
        vHallCards.insert(vHallCards.begin(), vWallCards.begin(), vWallCards.end());

        sortCard(vTempCarsd);
        sortCard(vHallCards);
        vecc_t vNumCards = nncard2cardnum(vTempCarsd);
        std::map<card_t, short> mCardCount = nncard2cardcount(vNumCards);
        if(vSrcCards.size() != 4)
        {
            return;
        }

        bool bFlush = true;
        int suit = nncard::getNNType(*vTempCarsd.begin());
        for(auto card : vTempCarsd)
        {
            if(suit != nncard::getNNType(card))
            {
                bFlush = false;
            }
        }

        if(mCardCount.size() == 4)
        {
            if(bFlush)
            {
                auto card = BestCardStraight(mCardCount, vHallCards, suit);
                if(card != -1)
                {
                    vDesCards.push_back(card);
                    return;
                }
                else//同花
                {
                    auto it = std::find_if(vHallCards.begin(), vHallCards.end(), [suit](card_t card)->bool{
                        return nncard::getNNType(card) == suit;
                    });
                    if(it != vHallCards.end())
                    {
                        vDesCards.push_back(*it);
                        return;
                    }
                }
            }
            else
            {
                auto card = BestCardStraight(mCardCount, vHallCards, -1);
                if(card != -1)
                {
                    vDesCards.push_back(card);
                    return;
                }
            }
        }
        else if(mCardCount.size() == 3)
        {
            auto it2 = std::find_if(mCardCount.begin(), mCardCount.end(), [](const std::pair<card_t, long>& item)->bool{
                    return item.second == 2;
            });
            auto itt = std::find_if(vHallCards.begin(), vHallCards.end(), [it2](card_t card)->bool{
                return nncard::getNNNum(card) ==it2->first;
            });
            if(itt != vHallCards.end())// 一对->三条
            {
                vDesCards.push_back(*itt);
                return;
            }
            for(auto rit = mCardCount.rbegin(); rit != mCardCount.rend(); rit++)//一对->两对
            {
                if(rit->second == 2)
                {
                    continue;
                }
                auto itt1 = std::find_if(vHallCards.begin(), vHallCards.end(), [rit](card_t card)->bool{
                    return nncard::getNNNum(card) == rit->first;
                });
                if(itt1 != vHallCards.end())
                {
                    vDesCards.push_back(*itt);
                    return;
                }
            }
        }
        else if(mCardCount.size() == 2) 
        {
            if(mCardCount.begin()->second == 1 || mCardCount.begin()->second == 3)
            {
                auto it3 = std::find_if(mCardCount.begin(), mCardCount.end(), [](const std::pair<card_t, long>& item)->bool{
                    return item.second == 3;
                });
                auto it1 = std::find_if(mCardCount.begin(), mCardCount.end(), [](const std::pair<card_t, long>& item)->bool{
                    return item.second == 1;
                });
                if(it1 == mCardCount.end() || it3 == mCardCount.end())
                {
                    return;
                }

                auto itt3 = std::find_if(vHallCards.begin(), vHallCards.end(), [it3](card_t card)->bool{
                        return nncard::getNNNum(card) == it3->first;
                });
                if(itt3 != vHallCards.end())//三条->四条
                {
                    vDesCards.push_back(*itt3);
                    return;
                }
                //三条->葫芦
                auto itt1 = std::find_if(vHallCards.begin(), vHallCards.end(), [it1](card_t card)->bool{
                    return nncard::getNNNum(card) == it1->first;
                });
                if(itt1 != vHallCards.end())
                {
                    vDesCards.push_back(*itt1);
                    return;
                }        
            }
            else//两对->葫芦
            {
                if(mCardCount.begin()->second != 2 || mCardCount.rbegin()->second != 2)
                {
                    return;
                }
                auto itt = std::find_if(vHallCards.begin(), vHallCards.end(), [mCardCount](card_t card)->bool{
                        return nncard::getNNNum(card) == mCardCount.rbegin()->first;
                });
                if(itt != vHallCards.end())
                {
                    vDesCards.push_back(*itt);
                    return;
                }
                auto itt2 = std::find_if(vHallCards.begin(), vHallCards.end(), [mCardCount](card_t card)->bool{
                    return nncard::getNNNum(card) == mCardCount.begin()->first;
                });
                if(itt2 != vHallCards.end())
                {
                    vDesCards.push_back(*itt2);
                    return;
                }
            }
        }
        else
        {
            vDesCards.push_back(vHallCards.back());
            return;
        }
    }

    bool compare(const vecc_t &vSrcCards1, const vecc_t &vSrcCards2)
    {
        if(vSrcCards1.size() != vSrcCards2.size() || vSrcCards1.size() < 1 || 
            vSrcCards1.size() > 4 || vSrcCards2.size() < 1 || vSrcCards2.size() > 4)
        {
            return vSrcCards1.size() < vSrcCards2.size();
        }

        vecc_t vTempCarsd1;
        vecc_t vTempCarsd2;
        vTempCarsd1.insert(vTempCarsd1.begin(), vSrcCards1.begin(), vSrcCards1.end());
        vTempCarsd2.insert(vTempCarsd2.begin(), vSrcCards2.begin(), vSrcCards2.end());
        
        sortCard(vTempCarsd1);
        sortCard(vTempCarsd2);
        vecc_t vNumCards1 = nncard2cardnum(vTempCarsd1);
        vecc_t vNumCards2 = nncard2cardnum(vTempCarsd2);
        std::map<card_t, short> mCardCount1 = nncard2cardcount(vNumCards1);
        std::map<card_t, short> mCardCount2 = nncard2cardcount(vNumCards2);

        if(vSrcCards1.size() == 1)
        {
            return compare_0p(vTempCarsd1, vTempCarsd2);
        }
        else if(vSrcCards1.size() == 2)
        {
            if(mCardCount1.size() != mCardCount2.size())
            {
                return mCardCount1.size() > mCardCount2.size();
            }
            if(mCardCount1.size() == 1)
            {
                return compare_1p(mCardCount1, mCardCount2, vTempCarsd1, vTempCarsd2); 
            }

            return compare_0p(vTempCarsd1, vTempCarsd2);
        }
        else if(vSrcCards1.size() == 3)
        {
            if(mCardCount1.size() != mCardCount2.size())
            {
                return mCardCount1.size() > mCardCount2.size();
            }
            if(mCardCount1.size() == 1)
            {
                auto it1 = std::find_if(mCardCount1.begin(), mCardCount1.end(), [](const std::pair<card_t, short>& item)->bool{
                    return item.second == 3;
                });
                auto it2 = std::find_if(mCardCount2.begin(), mCardCount2.end(), [](const std::pair<card_t, short>& item)->bool{
                    return item.second == 3;
                });

                if(it1 == mCardCount1.end() || it2 == mCardCount2.end())
                {
                    return false;
                }
                return it1->first < it2->first;
            }
            if(mCardCount1.size() == 2)
            {
               return compare_1p(mCardCount1, mCardCount2, vTempCarsd1, vTempCarsd2);
            }
            return compare_0p(vTempCarsd1, vTempCarsd2);
        }
        else if(vSrcCards1.size() == 4)
        {   
            if(mCardCount1.size() != mCardCount2.size())
            {
                return mCardCount1.size() > mCardCount2.size();
            }
            if(mCardCount1.size() == 1)
            {
                auto it1 = std::find_if(mCardCount1.begin(),mCardCount1.end(), [](const std::pair<card_t, short>& item)->bool{
                    return item.second == 4;
                });
                auto it2 = std::find_if(mCardCount2.begin(),mCardCount2.end(), [](const std::pair<card_t, short>& item)->bool{
                    return item.second == 4;
                });

                if(it1 != mCardCount1.end() || it2 != mCardCount2.end())
                {
                    return false;
                }
                else
                {
                    return it1->first < it2->first;
                }  
            }
            else if(mCardCount1.size() == 2)//三条 or 两对
            {
                auto it1 = std::find_if(mCardCount1.begin(),mCardCount1.end(), [](const std::pair<card_t, short>& item)->bool{
                    return item.second == 3;
                });
                auto it2 = std::find_if(mCardCount2.begin(),mCardCount2.end(), [](const std::pair<card_t, short>& item)->bool{
                    return item.second == 3;
                });

                if(it1 != mCardCount1.end() && it2 != mCardCount2.end())//三条
                {
                    return it1->first < it2->first;
                }
                else if(it1 != mCardCount1.end() || it2 != mCardCount2.end())
                {
                    return it2 != mCardCount1.end();
                }
                else //两对
                {
                    return compare_2p(mCardCount1, mCardCount2, vTempCarsd1, vTempCarsd2); 
                }
            }
            else if(mCardCount1.size() == 3)
            {
                return compare_1p(mCardCount1, mCardCount2, vTempCarsd1, vTempCarsd2); 
            }
            return compare_0p(vTempCarsd1, vTempCarsd2); 
        }
        else
        {
            return compare_0p(vTempCarsd1, vTempCarsd2); 
        }
    }

    bool compare(const vecc_t &vSrcCards1, const vecc_t &vSrcCards2, E_NN_TYPE nntype)
    {
        DLOG_TRACE("end compare num: "<<", nntype: "<< nntype);

        vecc_t vTempCarsd1;
        vecc_t vTempCarsd2;
        vTempCarsd1.insert(vTempCarsd1.begin(), vSrcCards1.begin(), vSrcCards1.end());
        vTempCarsd2.insert(vTempCarsd2.begin(), vSrcCards2.begin(), vSrcCards2.end());

        sortCard(vTempCarsd1);
        sortCard(vTempCarsd2);
        vecc_t vNumCards1 = nncard2cardnum(vTempCarsd1);
        vecc_t vNumCards2 = nncard2cardnum(vTempCarsd2);
        std::map<card_t, short> mCardCount1 = nncard2cardcount(vNumCards1);
        std::map<card_t, short> mCardCount2 = nncard2cardcount(vNumCards2);

        if (nntype == E_NN_TYPE::NN_TYPE_NN_STRAIGHT || nntype == E_NN_TYPE::NN_TYPE_NN_FLUSH 
            || nntype == E_NN_TYPE::NN_TYPE_NN_KING)
        {
            return compare_0p(vTempCarsd1, vTempCarsd2);
            // if(nncard::getNNNum(vTempCarsd1.back()) != nncard::getNNNum(vTempCarsd2.back()))
            // {
            //     return nncard::getNNNum(vTempCarsd1.back()) < nncard::getNNNum(vTempCarsd2.back());
            // }
            // else
            // {
            //     return nncard::getNNType(vTempCarsd1.back()) < nncard::getNNType(vTempCarsd2.back());
            // }
        }
        else if(nntype == E_NN_TYPE::NN_TYPE_NN_BOMB4)
        {
            auto it1 = std::find_if(mCardCount1.begin(), mCardCount1.end(), [](const std::pair<card_t, short>& item)->bool{
                return item.second == 4;
            });
            auto it2 = std::find_if(mCardCount2.begin(), mCardCount2.end(), [](const std::pair<card_t, short>& item)->bool{
                return item.second == 4;
            });
            if(it1 == mCardCount1.end() || it2 == mCardCount2.end())
            {
                return false;
            }
            else
            {
                return it1->first < it2->first;
            }
        }
        else if(nntype == E_NN_TYPE::NN_TYPE_NN_HULU || nntype == E_NN_TYPE::NN_TYPE_NN_BOMB3)
        {
            auto it1 = std::find_if(mCardCount1.begin(), mCardCount1.end(), [](const std::pair<card_t, short>& item)->bool{
                return item.second == 3;
            });
            auto it2 = std::find_if(mCardCount2.begin(), mCardCount2.end(), [](const std::pair<card_t, short>& item)->bool{
                return item.second == 3;
            });
            if(it1 == mCardCount1.end() || it2 == mCardCount2.end())
            {
                return false;
            }
            else
            {
                return it1->first < it2->first;
            }
        }
        else if(nntype == NN_TYPE_NN_SAMEKIND)
        {
            return nncard::getNNNum(vTempCarsd1.back()) < nncard::getNNNum(vTempCarsd2.back());
        }
        else if(nntype == NN_TYPE_NN_2P)
        {
           return compare_2p(mCardCount1, mCardCount2, vTempCarsd1, vTempCarsd2); 
        }
        else if(nntype == NN_TYPE_NN_1P)
        {
           return compare_1p(mCardCount1, mCardCount2, vTempCarsd1, vTempCarsd2);
        }
        else
        {
            return compare_0p(vTempCarsd1, vTempCarsd2);
        }
        return false;
    }

    bool compare_0p(const vecc_t &vSrcCards1, const vecc_t &vSrcCards2)
    {
        if(vSrcCards1.size() != vSrcCards2.size() || vSrcCards1.size() < 1 || vSrcCards2.size() < 1)
        {
            return false;
        }
      
        for(int i = vSrcCards1.size() -1; i >=0; i--)
        {
            DLOG_TRACE("compare_0p face1: "<< getNNNum(vSrcCards1[i]) <<", face2: "<< getNNNum(vSrcCards2[i]));
            if(nncard::getNNNum(vSrcCards1[i]) != nncard::getNNNum(vSrcCards2[i]))
            {
                return nncard::getNNNum(vSrcCards1[i]) < nncard::getNNNum(vSrcCards2[i]);
            }
        }
        for(int i = vSrcCards1.size() -1; i >=0; i--)
        {
            DLOG_TRACE("compare_0p suit1: "<< getNNNum(vSrcCards1[i]) <<", suit2: "<< getNNType(vSrcCards2[i]));
            if(nncard::getNNType(vSrcCards1[i]) != nncard::getNNType(vSrcCards2[i]))
            {
                return nncard::getNNType(vSrcCards1[i]) < nncard::getNNType(vSrcCards2[i]);
            }
        }
        return false;
    }

    bool compare_1p(std::map<card_t, short> mCardCount1, std::map<card_t, short> mCardCount2, vecc_t &vTempCarsd1, vecc_t &vTempCarsd2)
    {
        auto it1 = std::find_if(mCardCount1.begin(), mCardCount1.end(), [](const std::pair<card_t, short>& item)->bool{
                return item.second == 2;
        });
        auto it2 = std::find_if(mCardCount2.begin(), mCardCount2.end(), [](const std::pair<card_t, short>& item)->bool{
            return item.second == 2;
        });
        if(it1 == mCardCount1.end() || it2 == mCardCount2.end())
        {
            return false;
        }
        else
        {
            if(it1->first != it2->first)
            {
                return it1->first < it2->first;
            }
            else //删除对子
            {
                vecc_t vDelCarsd1;
                vecc_t vDelCarsd2;
                vDelCarsd1.insert(vDelCarsd1.begin(), vTempCarsd1.begin(), vTempCarsd1.end());
                vDelCarsd2.insert(vDelCarsd2.begin(), vTempCarsd2.begin(), vTempCarsd2.end());
                vecremove2(vDelCarsd1, it1->first);
                vecremove2(vDelCarsd2, it1->first);

                bool bSameFace = vDelCarsd1.size() == 0;
                if(!bSameFace)
                {
                    bSameFace = true;
                    for(unsigned int i = 0; i < vDelCarsd1.size(); i++)
                    {
                        if(getNNNum(vDelCarsd1[i]) != getNNNum(vDelCarsd2[i]))
                        {
                            bSameFace = false;
                            break;
                        }
                    }
                }
                DLOG_TRACE("compare_1p size: "<< vDelCarsd1.size());    

                if(bSameFace)
                {
                    return vTempCarsd1.end() == std::find_if(vTempCarsd1.begin(), vTempCarsd1.end(), [it1](card_t card)->bool{
                        return getNNType(card) == E_NN_CARD::NN_CARD_SPADE && it1->first == getNNNum(card);
                    });
                }
                return compare_0p(vDelCarsd1, vDelCarsd2);
            }
        }
    }

    bool compare_2p(std::map<card_t, short> mCardCount1, std::map<card_t, short> mCardCount2, vecc_t &vTempCarsd1, vecc_t &vTempCarsd2)
    {
        std::vector<short> vPairFace1;
        std::vector<short> vOneFace1;
        for(auto item : mCardCount1)
        {
            if(item.second == 2)
            {
                vPairFace1.push_back(item.first);
            }
            if(item.second == 1)
            {
                vOneFace1.push_back(item.first);
            }
        }
        std::vector<short> vPairFace2;
        std::vector<short> vOneFace2;
        for(auto item : mCardCount2)
        {
            if(item.second == 2)
            {
                vPairFace2.push_back(item.first);
            }
            if(item.second == 1)
            {
                vOneFace2.push_back(item.first);
            }
        }
        if(vPairFace1.size() != vPairFace2.size() || vOneFace1.size() != vOneFace2.size() || vPairFace1.size() == 0)
        {
            return false;
        }
        else
        {
            for(int i = vPairFace1.size() - 1; i >=0 ; i--)
            { 
                DLOG_TRACE("compare_2p pair face1 : "<< vPairFace1[i] << ", pair face2: "<< vPairFace2[i]); 
                if(vPairFace1[i] != vPairFace2[i])
                {
                    return vPairFace1[i] < vPairFace2[i];
                }
            }
            for(int i = vOneFace1.size() - 1; i >= 0; i--)
            { 
                DLOG_TRACE("compare_2p one face1 : "<< vPairFace1[i] << ", one face2: "<< vPairFace2[i]);
                if( vOneFace1[i] != vOneFace2[i])
                {
                    return vOneFace1[i] < vOneFace2[i];
                }
            }
        }
        // 最后比较最大对花色
        auto face = vPairFace1[0];
        auto it = std::find_if(vTempCarsd1.begin(), vTempCarsd1.end(), [face](card_t card)->bool{
            return face == nncard::getNNNum(card) && nncard::getNNType(card) == E_NN_CARD::NN_CARD_SPADE;
        });
        return it != vTempCarsd1.end();
    }

    E_NN_TYPE getnn(const vecc_t &vSrcCards)
    {
        if (vSrcCards.size() != 5)
        {
            return E_NN_TYPE::NN_TYPE_NN_N0;
        }

        //
        vecc_t vNumCards = nncard2cardnum(vSrcCards);
        std::map<card_t, short> mCardCount = nncard2cardcount(vNumCards);
        bool same = issamekind(vSrcCards);
        bool straight = isstraight(vNumCards, mCardCount, 4);
        if (same && straight)
        {
            std::map<card_t, short>::iterator fdA = mCardCount.find(14);
            std::map<card_t, short>::iterator fdK = mCardCount.find(13);
            if (fdA != mCardCount.end() && fdK != mCardCount.end())
            {
                return E_NN_TYPE::NN_TYPE_NN_KING;
            }
            else
            {
                return E_NN_TYPE::NN_TYPE_NN_FLUSH;
            }
        }

        int bombnum = getbombnum(mCardCount);
        if (bombnum >= 4)
        {
            return E_NN_TYPE::NN_TYPE_NN_BOMB4;
        }

        if (mCardCount.size() <= 2)
        {
            return E_NN_TYPE::NN_TYPE_NN_HULU;
        }

        if (same)
        {
            return E_NN_TYPE::NN_TYPE_NN_SAMEKIND;
        }

        if (straight)
        {
            return E_NN_TYPE::NN_TYPE_NN_STRAIGHT;
        }

        if (bombnum >= 3)
        {
            return E_NN_TYPE::NN_TYPE_NN_BOMB3;
        }

        if (mCardCount.size() <= 3)
        {
            return E_NN_TYPE::NN_TYPE_NN_2P;
        }

        if (bombnum >= 2)
        {
            return E_NN_TYPE::NN_TYPE_NN_1P;
        }

        return E_NN_TYPE::NN_TYPE_NN_N0;
    }

    std::map<card_t, short> nncard2cardcount(const vecc_t &vSrcCards)
    {
        std::map<card_t, short> mCardCount;
        for (auto it = vSrcCards.begin(); it != vSrcCards.end(); it++)
        {
            if (mCardCount.count(*it) > 0)
            {
                ++mCardCount[*it];
            }
            else
            {
                mCardCount[*it] = 1;
            }
        }

        return std::move(mCardCount);
    }

    vecc_t nncard2cardnum(const vecc_t &vSrcCards)
    {
        vecc_t vNumCards;
        for (auto it = vSrcCards.begin();it != vSrcCards.end();it++)
        {
            vNumCards.push_back(nncard::getNNNum(*it));
        }
        std::sort(vNumCards.begin(), vNumCards.end(), [](card_t c1, card_t c2)->bool{
            return c1 < c2;
        });
        return std::move(vNumCards);
    }

    void vecremove(vecc_t &vSrcCards1, vecc_t &vSrcCards2)
    {
        auto it = std::remove_if (vSrcCards1.begin(), vSrcCards1.end(), [vSrcCards2](card_t card1)->bool
        {
            return vSrcCards2.end() != std::find_if (vSrcCards2.begin(), vSrcCards2.end(), [card1](card_t card2)->bool
            {
                return getNNNum(card1) == getNNNum(card2) && getNNType(card1) == getNNType(card2);
            });
        });

        vSrcCards1.erase(it, vSrcCards1.end());
    }

    void vecremove2(vecc_t &vSrcCards, card_t face)
    {
        auto it = std::remove_if (vSrcCards.begin(), vSrcCards.end(), [face](card_t card1)->bool
        {
           return getNNNum(card1) == face;
        });

        vSrcCards.erase(it, vSrcCards.end());
    }
    bool checkcard(short card_num)
    {
        auto face = getNNNum(card_num);
        auto suit = getNNType(card_num);

        if(face < 2 || face > 14)
        {
            return false;
        }
        if((suit / 16) > 3 || (suit % 16) != 0)
        {
            return false;
        }
        return true;
    }
}