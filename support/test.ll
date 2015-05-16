; ModuleID = 'support/test.bc'
target datalayout = "e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@hw = global i8* getelementptr inbounds ([12 x i8]* @.str, i32 0, i32 0), align 8
@y = global i32 100, align 4
@.str1 = private unnamed_addr constant [17 x i8] c"Enter a number: \00", align 1
@.str2 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str3 = private unnamed_addr constant [17 x i8] c"You entered: %d\0A\00", align 1
@.str4 = private unnamed_addr constant [10 x i8] c"%s %d %u\0A\00", align 1
@.str5 = private unnamed_addr constant [10 x i8] c"%d %s %u\0A\00", align 1
@.str6 = private unnamed_addr constant [8 x i8] c"j: %d \0A\00", align 1
@bbCounter = internal global i32 0
@BasicBlockPrintfFormatStr = private constant [14 x i8] c"BB Count: %d\0A\00"

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
"0":
  %0 = load i32* @bbCounter
  %1 = add i32 1, %0
  store i32 %1, i32* @bbCounter
  %2 = alloca i32, align 4
  %x = alloca i32, align 4
  %j = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %2
  store i32 0, i32* %x, align 4
  %3 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([17 x i8]* @.str1, i32 0, i32 0))
  %4 = call i32 (i8*, ...)* @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8]* @.str2, i32 0, i32 0), i32* %x)
  %5 = load i32* %x, align 4
  %6 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([17 x i8]* @.str3, i32 0, i32 0), i32 %5)
  %7 = load i32* %x, align 4
  %8 = load i32* @y, align 4
  %9 = icmp ugt i32 %7, %8
  br i1 %9, label %"1", label %"2"

"1":                                              ; preds = %"0"
  %10 = load i32* @bbCounter
  %11 = add i32 1, %10
  store i32 %11, i32* @bbCounter
  %12 = load i8** @hw, align 8
  %13 = load i32* %x, align 4
  %14 = load i32* @y, align 4
  %15 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.str4, i32 0, i32 0), i8* %12, i32 %13, i32 %14)
  br label %"3"

"2":                                              ; preds = %"0"
  %16 = load i32* @bbCounter
  %17 = add i32 1, %16
  store i32 %17, i32* @bbCounter
  %18 = load i32* %x, align 4
  %19 = load i8** @hw, align 8
  %20 = load i32* @y, align 4
  %21 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.str5, i32 0, i32 0), i32 %18, i8* %19, i32 %20)
  br label %"3"

"3":                                              ; preds = %"2", %"1"
  %22 = load i32* @bbCounter
  %23 = add i32 1, %22
  store i32 %23, i32* @bbCounter
  store i32 0, i32* %j, align 4
  store i32 0, i32* %i, align 4
  br label %"4"

"4":                                              ; preds = %"6", %"3"
  %24 = load i32* @bbCounter
  %25 = add i32 1, %24
  store i32 %25, i32* @bbCounter
  %26 = load i32* %i, align 4
  %27 = load i32* %x, align 4
  %28 = icmp slt i32 %26, %27
  br i1 %28, label %"5", label %"7"

"5":                                              ; preds = %"4"
  %29 = load i32* @bbCounter
  %30 = add i32 1, %29
  store i32 %30, i32* @bbCounter
  %31 = load i32* %x, align 4
  %32 = load i32* %j, align 4
  %33 = add nsw i32 %32, %31
  store i32 %33, i32* %j, align 4
  br label %"6"

"6":                                              ; preds = %"5"
  %34 = load i32* @bbCounter
  %35 = add i32 1, %34
  store i32 %35, i32* @bbCounter
  %36 = load i32* %i, align 4
  %37 = add nsw i32 %36, 1
  store i32 %37, i32* %i, align 4
  br label %"4"

"7":                                              ; preds = %"4"
  %38 = load i32* @bbCounter
  %39 = add i32 1, %38
  store i32 %39, i32* @bbCounter
  %40 = load i32* %j, align 4
  %41 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([8 x i8]* @.str6, i32 0, i32 0), i32 %40)
  %42 = load i32* @bbCounter
  %43 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([14 x i8]* @BasicBlockPrintfFormatStr, i32 0, i32 0), i32 %42)
  ret i32 0
}

declare i32 @printf(i8*, ...) #1

declare i32 @__isoc99_scanf(i8*, ...) #1

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"Ubuntu clang version 3.4-1ubuntu3 (tags/RELEASE_34/final) (based on LLVM 3.4)"}
