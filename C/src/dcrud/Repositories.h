#pragma once
#include <dcrud/IRepositoryFactory.h>

void Repositories_publish(
   dcrudIRepositoryFactory * network,
   collSet *                updated, 
   collSet *                deleted  );
