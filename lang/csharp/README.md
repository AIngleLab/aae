# AIngle C# [![Build Status](https://travis-ci.org/apache/aingle.svg?branch=master)](https://travis-ci.org/apache/aingle) [![NuGet Package](https://img.shields.io/nuget/v/Apache.AIngle.svg)](https://www.nuget.org/packages/Apache.AIngle)

 [![AIngle](https://apache.aingle.ai/images/aingle-logo.png)](http://aingle.apache.org/)

 ## Install

 Install the Apache.AIngle package from NuGet:

 ```
Install-Package Apache.AIngle
```

## Build & Test

1. Install [.NET SDK 5.0+](https://dotnet.microsoft.com/download/dotnet-core)
2. `dotnet test`

## Dependency package version strategy

1. Use [`versions.props`](./versions.props) to specify package versions. `PackageReference` elements in `.csproj` files should use only version properties defined in [`versions.props`](./versions.props).

2. By updating the versions in our libraries, we require users of the library to update to a version equal to or greater than the version we reference. For example, if a user were to reference an older version of Newtonsoft.Json, they would be forced to update to a newer version before they could use a new version of the AIngle library.
In short, we should only update the version of the dependencies in our libraries if we absolutely must for functionality that we require. We leave it up to the users of the library as to whether or not they want the latest and greatest of a particularly dependency. We're only going to require the bare minimum.

## Notes

The [LICENSE](./LICENSE) and [NOTICE](./NOTICE) files in the lang/csharp source directory are used to build the binary distribution. The [LICENSE.txt](../../LICENSE.txt) and [NOTICE.txt](../../NOTICE.txt) information for the AIngle C# source distribution is in the root directory.
