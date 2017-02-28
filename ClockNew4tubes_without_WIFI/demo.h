//Программа Демо режима
//Шаги
//Внимание, данные очень сильно занимают память, не злоупотребляйте

static const uint8_t NumberAnimationArray[130][5] = 
{                                                 
                                                
    //Длительность каждого шага
    {8,10,10,10,400},
    {10,8,10,10,400},
    {10,10,8,10,400},
    {10,10,10,8,400},
    {10,10,8,10,400},
    {10,8,10,10,400},
    {8,10,10,10,350},
    {10,8,10,10,350},
    {10,10,8,10,350},
    {10,10,10,8,350},
    {10,10,8,10,350},
    {8,10,10,10,350},
    {10,8,10,10,300},
    {10,10,8,10,300},
    {10,10,10,8,300},
    {10,10,8,10,250},
    {10,8,10,10,250},
    {8,10,10,10,250},
    {10,8,10,10,250},
    {10,8,10,10,250},
    {8,10,10,10,1000},
    {9,10,10,10,400},
    {0,10,10,10,400},
    {1,10,10,10,400},
    {2,10,10,10,400},
    {3,10,10,10,400},
    {4,10,10,10,400},
    {5,10,10,10,400},
    {6,10,10,10,400},
    {7,10,10,10,400},
    {8,10,10,10,400},
    {7,10,10,10,400},
    {6,10,10,10,400},
    {5,10,10,10,400},
    {4,10,10,10,400},
    {3,10,10,10,400},
    {2,10,10,10,400},
    {1,10,10,10,400},
    {0,10,10,10,400},
    {9,10,10,10,400},
    {8,10,10,10,400},                                //след эффект
    {10,10,10,10,1000},
    {7,7,7,7,50},
    {10,10,10,10,800},
    {7,7,7,7,50},
    {10,10,10,10,800},
    {7,7,7,7,50},
    {10,10,10,10,800},
    {7,7,7,7,50},
    {10,10,10,10,800},
    {7,7,7,7,50},
    {10,10,10,10,1000},                                //след эффект
    {7,7,7,7,500},
    {10,7,7,7,500},
    {10,10,7,7,500},
    {10,10,10,7500},
    {7,10,10,10,500},
    {10,7,10,10,500},
    {10,10,7,10,500},
    {10,10,10,10,1000},                                 //след эффект кратковременное появление
    {3,10,10,10,200},
    {10,10,3,10,800},
    {10,10,10,10,200},
    {10,3,10,10,800},
    {10,10,10,3,200},
    {10,10,10,10,800},
    {3,3,3,3,1000},
    {10,10,10,10,1000},
    {3,3,3,3,1000},
    {10,10,10,10,1000},
    {3,3,3,3,1000},
    {10,10,10,10,1000},
    {3,3,3,3,1000},
    {10,10,10,10,1000},
    {9,10,10,10,300},
    {0,10,10,10,300},
    {1,10,10,10,300},
    {2,10,10,10,300},
    {3,10,10,10,300},
    {4,10,10,10,500},
    {10,4,10,10,500},
    {10,10,4,4,500},
    {10,4,10,10,500},
    {4,10,10,10,500},
    {10,4,10,10,500},
    {10,10,4,4,500},
    {10,10,10,4,500},
    {10,10,4,10,500},
    {10,10,10,4,500},
    {10,10,10,10,500},
    {10,10,10,10,500},
    {10,10,10,10,1000},
    {2,2,0,5,50},
    {2,2,1,5,50},
    {3,3,2,6,50},
    {3,3,3,6,50},
    {3,4,4,6,50},
    {4,4,5,7,50},
    {4,5,6,7,50},
    {4,5,7,7,50},
    {5,6,8,8,50},                                    //вращение на трех скорстях в двух напрявлениях
    {5,6,9,8,50},
    {5,7,0,8,50},
    {6,7,1,7,50},
    {6,8,2,7,50},
    {6,8,3,7,50},
    {7,9,4,6,50},
    {7,9,5,6,50},
    {7,0,6,6,50},
    {6,0,7,5,50},
    {6,1,8,5,50},
    {6,1,9,5,50},
    {5,2,0,4,50},
    {5,2,1,4,50},
    {5,3,2,4,50},
    {4,3,3,3,50},
    {4,4,4,3,50},
    {4,4,5,3,50},
    {3,5,6,2,50},
    {3,5,7,2,50},
    {3,6,8,2,50},
    {2,6,9,1,50},
    {2,7,0,1,50},
    {2,7,1,1,1000},
    {10,7,1,1,500},
    {10,10,1,1,500},
    {10,10,10,1,500},
    {10,10,10,10,500},
    {10,10,10,10,500},
    {10,10,10,10,3000}
};

