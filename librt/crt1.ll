declare i32 @atoi(i8* nocapture) nounwind

define i32 @main(i32 %argc, i8** %argv) {
entry:
    %0 = getelementptr i8*, i8** %argv, i64 1
    %1 = load i8*, i8** %0
    %2 = call i32 @atoi(i8* %1)
    %3 = call i32 @entry(i32 %2)
    ret i32 %3
}

