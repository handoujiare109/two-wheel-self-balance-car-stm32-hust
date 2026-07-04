主项目在Template目录里，里面有readme文件。
aa-ee，session，todo为调试过程记录文件。可以不部署。
在主目录里的motor.c和exti.c为平衡小车给的示例文件，不是项目所用的文件。项目文件都在template目录里。
如果需要比readme更详细的项目解释，可以参照v1.md和汇报文件。
各项目的readme文件可能跟最终代码有出入。调试的时候比较混乱。最后一个完成的文件是v1.md，里面的内容应该跟最终代码一致。
test用来检验编译烧录环境。test2用来测试电机死区。
最终代码部署之后，只需要根据小车表现调整./Template/HARDWARE/CONTROL/control.c里的pid参数就能完成项目任务的基本内容。
