# AIngle C# [![Build Status](https://travis-ci.com/AIngleLab/aingle.svg?branch=master)](https://travis-ci.com/AIngleLab/aingle) [![NuGet Package](https://img.shields.io/nuget/v/AIngle.Apache.svg)](https://www.nuget.org/packages/AIngle.Apache)

 [![AIngle](https://apache.aingle.ai/images/aingle-logo.png)](http://apache.aingle.ai/)

 ## Install

 Install the AIngle.Apache package from NuGet:

 ```
Install-Package AIngle.Apache
```

## Build & Test

1. Install [.NET SDK 5.0+](https://dotnet.microsoft.com/download/dotnet-core)
2. `dotnet test`

## Dependency package version strategy

1. Use [`versions.props`](./versions.props) to specify package versions. `PackageReference` elements in `.csproj` files should use only version properties defined in [`versions.props`](./versions.props).

2. By updating the versions in our libraries, we require users of the library to update to a version equal to or greater than the version we reference. For example, if a user were to reference an older version of Newtonsoft.Json, they would be forced to update to a newer version before they could use a new version of the AIngle library.
In short, we should only update the version of the dependencies in our libraries if we absolutely must for functionality that we require. We leave it up to the users of the library as to whether or not they want the latest and greatest of a particularly dependency. We're only going to require the bare minimum.

