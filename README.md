# Transfer-Function-Tool
vulkan volume rendering with 2d-tf (chart)

本程序的目的：用来辅助体绘制过程中2D传输函数的设计

## 现有的体绘制的工具

当前关于体绘制使用比较多的工具主要有两个 ———— Voreen 和 Paraview 

### Paraview 

advantages：

    1. Paraview是一个主要使用VTK编写的开源工具，功能非常强大,包括了三维数据分析所需的(包括体绘制等)分析所需的大部分功能
    
    2. 界面设计很直观，不需要太多时间去熟悉和学习，上手很快

disadvantages：

    1. 其中关于传输函数的设计，只提供了一维传输函数的设计，无法设计更高维的传输函数。
    
    2. 虽然Find Data的功能可以通过选择属性参数值的范围，来选中对应的数据。但是无法实现通过两个属性之间比较关系(比如大于，小于)来选中对应数据。
   
       (如想选中属性A的值大于属性B的值所对应得数据部分，软件中没有直接的提供这个功能)

### Voreen

advantages：

    1. 提供了一种二维传输函数设计的交互界面。
    
    2. 二维传输函数的交互非常方便，提供了三种primitive去选中传输函数域中感兴趣的区域以及包含了很多其他基本功能
    
    3. 传输函数交互设计的结果可以很快获得，直观而且反馈的时间很快。

disadvantages：

    1. 用户界面设计不如paraview直观，需要一点时间去熟悉使用
    
    2. 所提供的二维传输函数域仅有一种(scalar-gradient)，固定了横纵坐标的属性。
    
    
### 本工具

功能：可以自己选择二维传输函数域的横纵坐标属性，并生成对应的散点图，可以通过add primitive来选中自己感兴趣的区域，选中部分对应的数据的体绘制结果会在vulkanWindow界面中显示。

目标文件类型：.vtm file，可以一次性读取多个文件

实现方法：数据读取通过vtk实现，散点图的绘制基于QT自带的QChart库，体绘制结果的显示基于QT自带的QVulaknWindow
  


