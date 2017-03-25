//
//  Functional.hpp
//  entitas-demo
//
//  Created by Igor Makaruks on 24/03/2017.
//  Copyright © 2017 Igor Makaruks. All rights reserved.
//

#ifndef Functional_h
#define Functional_h

template <typename Collection,typename unop>
inline void for_each(Collection col, unop op){
    std::for_each(col.begin(), col.end(), op);
}


template <typename Collection, typename E>
inline bool doesExist(Collection col, E element){
    return std::find(col.begin(), col.end(), element) != col.end();
}

#endif /* Functional_h */
