//Программа Демо режима
//Шаги



static const uint8_t NumberAnimationArray[130][7] = 
{                                                 
    //Длительность каждого шага
    {8,10,10,10,10,10,8},
    {10,8,10,10,10,10,8},
    {10,10,8,10,10,10,8},
    {10,10,10,8,10,10,8},
    {10,10,10,10,8,10,8},
    {10,10,10,10,10,8,8},
    {10,10,10,10,8,10,7},
    {10,10,10,8,10,10,7},
    {10,10,8,10,10,10,7},
    {10,8,10,10,10,10,7},
    {8,10,10,10,10,10,7},
    {10,8,10,10,10,10,6},
    {10,10,8,10,10,10,6},
    {10,10,10,8,10,10,6},
    {10,10,10,10,8,10,6},
    {10,10,10,10,10,8,6},
    {10,10,10,10,8,10,6},
    {10,10,10,8,10,10,6},
    {10,10,8,10,10,10,6},
    {10,8,10,10,10,10,6},
    {8,10,10,10,10,10,20},
    {9,10,10,10,10,10,8},
    {0,10,10,10,10,10,8},
    {1,10,10,10,10,10,8},
    {2,10,10,10,10,10,8},
    {3,10,10,10,10,10,8},
    {4,10,10,10,10,10,8},
    {5,10,10,10,10,10,8},
    {6,10,10,10,10,10,8},
    {7,10,10,10,10,10,8},
    {8,10,10,10,10,10,8},
    {7,10,10,10,10,10,8},
    {6,10,10,10,10,10,8},
    {5,10,10,10,10,10,8},
    {4,10,10,10,10,10,8},
    {3,10,10,10,10,10,8},
    {2,10,10,10,10,10,8},
    {1,10,10,10,10,10,8},
    {0,10,10,10,10,10,8},
    {9,10,10,10,10,10,8},
    {8,10,10,10,10,10,8},                                //след эффект
    {10,10,10,10,10,10,20},
    {7,7,7,7,7,7,1},
    {10,10,10,10,10,10,16},
    {7,7,7,7,7,7,1},
    {10,10,10,10,10,10,16},
    {7,7,7,7,7,7,1},
    {10,10,10,10,10,10,16},
    {7,7,7,7,7,7,1},
    {10,10,10,10,10,10,16},
    {7,7,7,7,7,7,1},
    {10,10,10,10,10,10,20},                                //след эффект
    {7,7,7,7,7,7,10},
    {10,7,7,7,7,7,10},
    {10,10,7,7,7,7,10},
    {10,10,10,7,7,7,10},
    {10,10,10,10,7,7,10},
    {10,10,10,10,10,7,10},
    {10,10,10,10,10,10,10},
    {10,10,10,10,10,10,20},                                 //след эффект кратковременное появление
    {3,10,10,10,10,10,4},
    {10,10,3,10,10,10,16},
    {10,10,10,10,3,10,4},
    {10,3,10,10,10,10,16},
    {10,10,10,3,10,10,4},
    {10,10,10,10,10,3,16},
    {3,3,3,3,3,3,20},
    {10,10,10,10,10,10,20},
    {3,3,3,3,3,3,20},
    {10,10,10,10,10,10,20},
    {3,3,3,3,3,3,20},
    {10,10,10,10,10,10,20},
    {3,3,3,3,3,3,20},
    {10,10,10,10,10,10,20},
    {9,10,10,10,10,9,6},
    {0,10,10,10,10,8,6},
    {1,10,10,10,10,7,6},
    {2,10,10,10,10,6,6},
    {3,10,10,10,10,5,6},
    {4,10,10,10,10,4,10},
    {10,4,10,10,4,10,10},
    {10,10,4,4,10,10,10},
    {10,4,10,10,4,10,10},
    {4,10,10,10,10,4,10},
    {10,4,10,10,4,10,10},
    {10,10,4,4,10,10,10},
    {10,10,10,4,10,10,10},
    {10,10,4,10,10,10,10},
    {10,10,10,4,10,10,10},
    {10,10,10,10,4,10,10},
    {10,10,10,10,10,4,10},
    {10,10,10,10,10,10,20},
    {2,2,0,5,3,7,1},
    {2,2,1,5,3,8,1},
    {3,3,2,6,4,9,1},
    {3,3,3,6,4,0,1},
    {3,4,4,6,5,1,1},
    {4,4,5,7,5,2,1},
    {4,5,6,7,6,3,1},
    {4,5,7,7,6,4,1},
    {5,6,8,8,7,5,1},                                    //вращение на трех скорстях в двух напрявлениях
    {5,6,9,8,7,6,1},
    {5,7,0,8,8,7,1},
    {6,7,1,7,8,6,1},
    {6,8,2,7,7,5,1},
    {6,8,3,7,7,4,1},
    {7,9,4,6,6,3,1},
    {7,9,5,6,6,2,1},
    {7,0,6,6,5,1,1},
    {6,0,7,5,5,0,1},
    {6,1,8,5,4,9,1},
    {6,1,9,5,4,8,1},
    {5,2,0,4,3,7,1},
    {5,2,1,4,3,6,1},
    {5,3,2,4,2,5,1},
    {4,3,3,3,2,4,1},
    {4,4,4,3,1,3,1},
    {4,4,5,3,1,2,1},
    {3,5,6,2,0,1,1},
    {3,5,7,2,0,0,1},
    {3,6,8,2,9,9,1},
    {2,6,9,1,9,8,1},
    {2,7,0,1,8,7,1},
    {2,7,1,1,8,6,20},
    {10,7,1,1,8,6,10},
    {10,10,1,1,8,6,10},
    {10,10,10,1,8,6,10},
    {10,10,10,10,8,6,10},
    {10,10,10,10,10,6,10},
    {10,10,10,10,10,10,60}
};


