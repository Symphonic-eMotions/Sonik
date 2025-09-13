//
//  RNBOList.m
//  SwiftRNBO_Example_multiplatfrom_SwiftUI
//
//  Created by Эльдар Садыков on 22.02.2023.
//

#include "RNBO.h"
#import "RNBOListKarplusStrong.h"

@implementation RNBOListKarplusStrong{
    std::shared_ptr<RNBO::list> _obj;
}

- (void)fromArrayOfNumbers:(NSArray<NSNumber *> *)array {
}

- (NSArray<NSNumber *> *)toArrayOfNumbers {
    return @[];
}

@end

#pragma mark -

@interface RNBOListKarplusStrong (CxxCounterpart)
- (void)setCxxList:(std::shared_ptr<RNBO::list>)list;
@end

@implementation RNBOListKarplusStrong (CxxCounterpart)
- (void)setCxxList:(std::shared_ptr<RNBO::list>)list {
    self->_obj = list;
}

@end
