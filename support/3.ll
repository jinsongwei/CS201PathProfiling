; ModuleID = 'support/3.bc'
target datalayout = "e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@bbCounter = internal global i32 0
@BasicBlockPrintfFormatStr = private constant [14 x i8] c"BB Count: %d\0A\00"

; Function Attrs: nounwind uwtable
define void @function_1(i32 %x) #0 {
"0":
  %0 = load i32* @bbCounter
  %1 = add i32 1, %0
  store i32 %1, i32* @bbCounter
  %2 = alloca i32, align 4
  %y = alloca i32, align 4
  %z = alloca i32, align 4
  store i32 %x, i32* %2, align 4
  %3 = load i32* %2, align 4
  store i32 %3, i32* %y, align 4
  %4 = load i32* %2, align 4
  store i32 %4, i32* %z, align 4
  br label %"1"

"1":                                              ; preds = %"5", %"0"
  %5 = load i32* @bbCounter
  %6 = add i32 1, %5
  store i32 %6, i32* @bbCounter
  %7 = load i32* %y, align 4
  %8 = icmp ugt i32 %7, 0
  br i1 %8, label %"2", label %"6"

"2":                                              ; preds = %"1"
  %9 = load i32* @bbCounter
  %10 = add i32 1, %9
  store i32 %10, i32* @bbCounter
  %11 = load i32* %2, align 4
  store i32 %11, i32* %z, align 4
  br label %"3"

"3":                                              ; preds = %"4", %"2"
  %12 = load i32* @bbCounter
  %13 = add i32 1, %12
  store i32 %13, i32* @bbCounter
  %14 = load i32* %z, align 4
  %15 = icmp ugt i32 %14, 0
  br i1 %15, label %"4", label %"5"

"4":                                              ; preds = %"3"
  %16 = load i32* @bbCounter
  %17 = add i32 1, %16
  store i32 %17, i32* @bbCounter
  %18 = load i32* %z, align 4
  %19 = add i32 %18, -1
  store i32 %19, i32* %z, align 4
  br label %"3"

"5":                                              ; preds = %"3"
  %20 = load i32* @bbCounter
  %21 = add i32 1, %20
  store i32 %21, i32* @bbCounter
  %22 = load i32* %y, align 4
  %23 = add i32 %22, -1
  store i32 %23, i32* %y, align 4
  br label %"1"

"6":                                              ; preds = %"1"
  %24 = load i32* @bbCounter
  %25 = add i32 1, %24
  store i32 %25, i32* @bbCounter
  ret void
}

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
"0":
  %0 = load i32* @bbCounter
  %1 = add i32 1, %0
  store i32 %1, i32* @bbCounter
  %2 = alloca i32, align 4
  store i32 0, i32* %2
  call void @function_1(i32 10)
  %3 = load i32* @bbCounter
  %4 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([14 x i8]* @BasicBlockPrintfFormatStr, i32 0, i32 0), i32 %3)
  ret i32 0
}

declare i32 @printf(i8*, ...)

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"Ubuntu clang version 3.4-1ubuntu3 (tags/RELEASE_34/final) (based on LLVM 3.4)"}
